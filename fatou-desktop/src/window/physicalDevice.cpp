#include "physicalDevice.h"

#include <iostream>

#include "gui/cef/js.h"

bool PhysicalDevice::checkDeviceExtensionSupport() const {
    uint32_t extensionCount;

    vector<vk::ExtensionProperties> availableExtensions =
        device.enumerateDeviceExtensionProperties();

    std::set<string> requiredExtensions(deviceExtensions.begin(),
                                        deviceExtensions.end());

    for (const auto &extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices PhysicalDevice::findQueueFamilies() const {
    QueueFamilyIndices indices;

    // Assign index to queue families that could be found

    vector<vk::QueueFamilyProperties> queueFamilies =
        device.getQueueFamilyProperties();

    // We need to find at least one queue family that supports
    // VK_QUEUE_GRAPHICS_BIT.
    int i = 0;
    for (const auto &queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics &&
            queueFamily.timestampValidBits > 0) {
            indices.graphicsFamily = i;
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute &&
            queueFamily.timestampValidBits > 0) {
            indices.computeFamily = i;
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) {
            // choose a transfer queue (TODO: maybe choose one that is not the
            // graphics queue to allow the driver to focus on moving stuff? But
            // it requires shared ressource then. And also I have to make sure
            // there is enough synchronization when doing layout transitions. Is
            // this still fine? Does it pay off?)
            indices.transferFamily = i;
        }

        vk::Bool32 presentSupport =
            device.getSurfaceSupportKHR(i, win->getSurface());

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }

        i++;
    }

    return indices;
}

SwapChainSupportDetails PhysicalDevice::querySwapChainSupport() const {
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/Swap_chain

    SwapChainSupportDetails details;

    auto surface = win->getSurface();

    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
    details.formats = device.getSurfaceFormatsKHR(surface);
    details.presentModes = device.getSurfacePresentModesKHR(surface);

    return details;
}

void jsFeedback(const vk::PhysicalDeviceProperties &deviceProperties, int score,
                const string &reason) {
    commitJS("appendDevice", {{"name", jsStr(deviceProperties.deviceName)},
                              {"score", jsStr(score)},
                              {"reason", jsStr(reason)}});
    if (reason.length() > 0)
        std::cout << " rejected due to " << reason << std::endl;
}

int PhysicalDevice::rateDeviceSuitability() const {

    const vk::PhysicalDeviceProperties deviceProperties =
        device.getProperties();
    const vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

    std::cout << deviceProperties.deviceName;

    int score = 0;

    if (!deviceFeatures.samplerAnisotropy) {
        jsFeedback(deviceProperties, 0, "no anisotropy sampler");
        return 0;
    }

    QueueFamilyIndices indices = findQueueFamilies();
    if (!indices.isComplete()) {
        jsFeedback(deviceProperties, 0, "missing queue family");
        return 0;
    }

    if (!checkDeviceExtensionSupport()) {
        jsFeedback(deviceProperties, 0, "missing extension");
        return 0;
    } else {
        // It is important that we only try to query for swap chain support
        // after verifying that the extension is available.
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
        if (swapChainSupport.formats.empty() ||
            swapChainSupport.presentModes.empty()) {
            jsFeedback(deviceProperties, 0, "missing presentation mode");
            return 0;
        }
    }

    // Score

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        jsFeedback(deviceProperties, 0, "no geometry shader support");
        return 0;
    }

    std::cout << " scored " << score << std::endl;
    jsFeedback(deviceProperties, score, "");
    return score;
}

vector<shared_ptr<PhysicalDevice>>
PhysicalDevice::getPhysicalDevices(shared_ptr<NativeWindow> win) {

    vk::raii::Instance const &inst = win->getInstance()->instance;

    vector<vk::raii::PhysicalDevice> devices = inst.enumeratePhysicalDevices();
    if (devices.size() == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    vector<shared_ptr<PhysicalDevice>> devicesEx;
    for (const auto device : devices) {
        devicesEx.push_back(make_shared<PhysicalDevice>(win, device));
    }
    return devicesEx;
}

shared_ptr<PhysicalDevice>
PhysicalDevice::pickPhysicalDevice(shared_ptr<NativeWindow> win) {
    auto devices = PhysicalDevice::getPhysicalDevices(win);

    std::multimap<int, shared_ptr<PhysicalDevice>> candidates;
    for (auto &device : devices) {
        int score = device->rateDeviceSuitability();
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0) {
        return candidates.rbegin()->second;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

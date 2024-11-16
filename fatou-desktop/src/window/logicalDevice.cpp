#include "logicalDevice.h"

vk::raii::Device LogicalDevice::createDevice() {
    // https://vulkan-tutorial.com/en/Drawing_a_triangle/Setup/Logical_device_and_queues

    vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                              indices.presentFamily.value(),
                                              indices.transferFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = vk::StructureType::eDeviceQueueCreateInfo;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    ///////////////////////////
    // create the logical device

    vk::DeviceCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eDeviceCreateInfo;
    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    const void **pNext = &createInfo.pNext;

    /*
    // enable the host query reset feature (for auto resetting timer
    queries) vk::PhysicalDeviceHostQueryResetFeatures resetFeatures;
    resetFeatures.sType =
        vk::StructureType::ePhysicalDeviceHostQueryResetFeatures;
    resetFeatures.hostQueryReset = VK_TRUE;
    *pNext = &resetFeatures;
    pNext = (const void**)&resetFeatures.pNext;
    */

    /*
    // Much more features in 11, 12, 13 etc...
    vk::PhysicalDeviceVulkan11Features vulkan11features{};
    *pNext = &vulkan11features;
    pNext = (const void **)&vulkan11features.pNext;
    */

    // we want anisotropy and 64 bits
    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.shaderFloat64 = VK_TRUE;
    deviceFeatures.shaderInt64 = VK_TRUE;
    deviceFeatures.shaderInt16 = VK_TRUE;
    createInfo.pEnabledFeatures = &deviceFeatures;

    // enable extensions
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(physical->deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = physical->deviceExtensions.data();

    auto instance = physical->win->getInstance();

    if (instance->enableValidationLayers) {
        // devices specific validation layers are ignored in modern Vulkan
        // drivers
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(instance->validationLayers.size());
        createInfo.ppEnabledLayerNames = instance->validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    // TODO: does this work without destroying it?
    return physical->device.createDevice(createInfo);
}

#include "vulkanInstance.h"
#include <iostream>

#include <boost/algorithm/string/join.hpp>

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
              void *pUserData) {

    /*
    messageSeverity
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the
    creation of a resource VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    Message about behavior that is not necessarily an error, but very likely a
    bug in your application VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    Message about behavior that is invalid and may cause crashes

    messageType
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: Some event has happened that is
    unrelated to the specification or performance
    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: Something has happened that
    violates the specification or indicates a possible mistake
    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: Potential non-optimal use
    of Vulkan

    */

    if ((int)messageSeverity >=
        (int)vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        // Message is important enough to show
        std::cerr << "validation layer: " << pCallbackData->pMessage
                  << std::endl;
    }

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
        // vk::Result::eErrorExtensionNotPresent;
    }
}

std::set<string> get_supported_extensions() {
    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count,
                                           nullptr); // get number of extensions
    vector<vk::ExtensionProperties> extensions =
        vk::enumerateInstanceExtensionProperties();
    std::set<string> results;
    for (auto &extension : extensions) {
        results.insert(extension.extensionName);
    }
    return results;
}

void printVulkanInfo() {

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";
    std::cout << boost::algorithm::join(get_supported_extensions(), "\n")
              << std::endl;
}

void populateDebugMessengerCreateInfo(
    vk::DebugUtilsMessengerCreateInfoEXT &createInfo) {
    // createInfo = {};
    createInfo.sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
    createInfo.messageSeverity =
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
    createInfo.messageType =
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional
}

void VulkanInstance::updateRequiredExtensions() {

    if (enableValidationLayers) {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
}

VulkanInstance::VulkanInstance(vector<const char *> requiredExtensions)
    : requiredExtensions(requiredExtensions), instance(createInstance()),
      debugMessenger(nullptr) {
    setupDebugMessenger();
}

vk::raii::Instance VulkanInstance::createInstance() {

    printVulkanInfo();

    updateRequiredExtensions();

    if (enableValidationLayers && !checkValidationLayerSupport()) {
        std::cout << "validation not avail" << std::endl;
        throw std::runtime_error(
            "validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{};
    appInfo.sType = vk::StructureType::eApplicationInfo;
    appInfo.pApplicationName = "Hello Triangle"; // TODO
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    vk::InstanceCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eInstanceCreateInfo;
    createInfo.pApplicationInfo = &appInfo;

    // Enable validation layers

    // created here to ensure it is not destroyed before vkCreateInstance
    // By creating an additional debug messenger this way it will
    // automatically be used during vkCreateInstance and vkDestroyInstance
    // and cleaned up after that.

    if (enableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext =
            (vk::DebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    // Enable extensions

    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    return vk::raii::Instance(context, createInfo);
    /*
        vk::Result result = vkCreateInstance(&createInfo, nullptr, &instance);
        switch (result) {
        case VK_SUCCESS:
            std::cout << "Vulkan successfully initialized" << std::endl;
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            throw std::runtime_error("Vulkan extension not presents");
        default:
            throw std::runtime_error("failed to create instance!");
        }*/
}

void VulkanInstance::setupDebugMessenger() {
    if (!enableValidationLayers)
        return;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    debugMessenger = instance.createDebugUtilsMessengerEXT(createInfo);
}

bool VulkanInstance::checkValidationLayerSupport() {
    vector<vk::LayerProperties> availableLayers =
        vk::enumerateInstanceLayerProperties();

    for (const char *layerName : validationLayers) {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

VulkanInstance::~VulkanInstance() {
    std::cout << "Destroy vulkan instance" << std::endl;

    /*if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }*/
    // vkDestroyInstance(instance, nullptr);
}
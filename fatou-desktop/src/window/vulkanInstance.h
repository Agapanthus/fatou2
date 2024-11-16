#pragma once

#include <iostream>

class VulkanInstance : public std::enable_shared_from_this<VulkanInstance>,
                       private boost::noncopyable {
  public:
    VulkanInstance(vector<const char *> requiredExtensions);

    vk::Instance get() { return *instance; }
    ~VulkanInstance();

  private:
    void setupDebugMessenger();
    bool checkValidationLayerSupport();
    void updateRequiredExtensions();
    vk::raii::Instance createInstance();

  public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

  private:
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    vector<const char *> requiredExtensions;

    vk::raii::Context context;

  public:
    const vector<const char *> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};

    // TODO: Private
    vk::raii::Instance instance;

  private:
    vk::raii::DebugUtilsMessengerEXT debugMessenger;
};
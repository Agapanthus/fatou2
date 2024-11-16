#pragma once

#include "nativeWindow.h"

struct QueueFamilyIndices {

    // compute + timing
    optional<uint32_t> computeFamily;

    // graphics + timing
    optional<uint32_t> graphicsFamily;

    // It's possible that the queue families supporting drawing commands and
    // the ones supporting presentation do not overlap.
    optional<uint32_t> presentFamily;

    // for copying data
    optional<uint32_t> transferFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value() &&
               transferFamily.has_value() && computeFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    vector<vk::SurfaceFormatKHR> formats;
    vk::SurfaceCapabilitiesKHR capabilities;
    vector<vk::PresentModeKHR> presentModes;
};

class PhysicalDevice : private boost::noncopyable {
  public:
    PhysicalDevice(shared_ptr<NativeWindow> win,
                   vk::raii::PhysicalDevice const &device)
        : device(device), win(win), properties(device.getProperties()) {}
    int rateDeviceSuitability() const;
    QueueFamilyIndices findQueueFamilies() const;
    SwapChainSupportDetails querySwapChainSupport() const;

    static vector<shared_ptr<PhysicalDevice>>
    getPhysicalDevices(shared_ptr<NativeWindow> win);

    static shared_ptr<PhysicalDevice>
    pickPhysicalDevice(shared_ptr<NativeWindow> win);

    shared_ptr<NativeWindow> getWindow() { return win; }
    vk::PhysicalDevice handle() { return *device; }
    
  public:
    const vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  private:
    bool checkDeviceExtensionSupport() const;

  public:
    vk::raii::PhysicalDevice device;
    const vk::PhysicalDeviceProperties properties;

  private:
    shared_ptr<NativeWindow> win;

    friend class LogicalDevice;
};
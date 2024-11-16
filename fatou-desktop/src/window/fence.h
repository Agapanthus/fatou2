#pragma once
#include "logicalDevice.h"

class Fence : private boost::noncopyable {
  public:
    Fence(shared_ptr<LogicalDevice> device, bool signaled = false)
        : device(device), handle(nullptr) {
        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.sType = vk::StructureType::eFenceCreateInfo;
        fenceInfo.flags = {};
        if (signaled)
            fenceInfo.flags |= vk::FenceCreateFlagBits::eSignaled;
        handle = device->device.createFence(fenceInfo);
    }

    void wait(uint64_t timeout = UINT64_MAX) {
        vk::Result result =
            device->device.waitForFences({*handle}, true, timeout);
        if (result == vk::Result::eTimeout) {
            // TODO
        }
    }

    void reset() { device->device.resetFences({*handle}); }

    ~Fence() { // vkDestroyFence(device->handle(), handle, nullptr);
    }

    vk::raii::Fence handle;

  private:
    shared_ptr<LogicalDevice> device;
};
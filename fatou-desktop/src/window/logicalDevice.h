#pragma once

#include "physicalDevice.h"
#include <iostream>
class LogicalDevice : private boost::noncopyable {
  public:
    LogicalDevice(shared_ptr<PhysicalDevice> physicalDevice)
        : physical(physicalDevice), indices(physical->findQueueFamilies()),
          device(createDevice()),
          // Because we're only creating a single queue from this family, we'll
          // simply use index 0.
          transferQueue(device.getQueue(indices.transferFamily.value(), 0)),
          graphicsQueue(device.getQueue(indices.graphicsFamily.value(), 0)),
          presentQueue(device.getQueue(indices.presentFamily.value(), 0)) {}

    ~LogicalDevice() { std::cout << "Destroy Logical device..." << std::endl; }

    vk::Device handle() const { return static_cast<vk::Device>(*device); }

    void waitIdle() { device.waitIdle(); }

  public:
    // TODO: private!
    const shared_ptr<PhysicalDevice> physical;
    const QueueFamilyIndices indices;
    const vk::raii::Device device;
    const vk::raii::Queue graphicsQueue;
    const vk::raii::Queue presentQueue;
    const vk::raii::Queue transferQueue;

  private:
    vk::raii::Device createDevice();
};
#pragma once
#include "logicalDevice.h"

// - It's important to preserve the order of some operations
// - semaphores can synchronize intra and inter queue on the GPU
// - Semaphores can be binary or timeline, signaled or unsignaled
// - use Semaphores like: one operation sends the signal and the other waits
// for the signal
// - Fences synchronize on the CPU

class Semaphore : private boost::noncopyable {

  public:
    Semaphore(shared_ptr<LogicalDevice> device) : device(device), handle(nullptr) {
        vk::SemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = vk::StructureType::eSemaphoreCreateInfo;
        handle= device->device.createSemaphore(semaphoreInfo);
       
    }

    ~Semaphore() { //vkDestroySemaphore(device->handle(), handle, nullptr); 
    }

    vk::raii::Semaphore handle;

  private:
    shared_ptr<LogicalDevice> device;
};
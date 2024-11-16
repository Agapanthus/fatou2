#include "buffer.h"

inline void createBuffer(const LogicalDevice *device, const vk::DeviceSize size,
                         const vk::BufferUsageFlags usage, vk::raii::Buffer &buffer) {
    vk::BufferCreateInfo bufferInfo{};
    bufferInfo.sType = vk::StructureType::eBufferCreateInfo;

    bufferInfo.size = size;

    // The second field is usage, which indicates for which purposes the data in
    // the buffer is going to be used. It is possible to specify multiple
    // purposes using a bitwise or.
    bufferInfo.usage = usage;

    // Just like the images in the swap chain, buffers can also be owned by a
    // specific queue family or be shared between multiple at the same time.
    QueueFamilyIndices indices = device->physical->findQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.transferFamily.value()};

    if (indices.graphicsFamily != indices.transferFamily) {
        // vk::SharingMode::eConcurrent: Resources can be used across multiple
        // queue families without explicit ownership transfers.
        std::cout << "Warning: Using concurrent queues for vertex buffer. "
                     "Performance might be suboptimal."
                  << std::endl;
        bufferInfo.sharingMode = vk::SharingMode::eConcurrent;
        bufferInfo.queueFamilyIndexCount = 2;
        bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        // vk::SharingMode::eExclusive: a resource is owned by one queue family at
        // a time and ownership must be explicitly transferred before using it
        // in another queue family. This option offers the best performance.
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;
        bufferInfo.queueFamilyIndexCount = 0;     // Optional
        bufferInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    // The flags parameter is used to configure sparse buffer memory, which is
    // not relevant right now.
    bufferInfo.flags = {};

    buffer = device->device.createBuffer(bufferInfo);
}

inline void allocateMemory(const LogicalDevice *device,
                           const vk::MemoryPropertyFlags properties,
                           const vk::raii::Buffer &buffer, vk::raii::DeviceMemory &memory) {
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(device, memRequirements.memoryTypeBits, properties);

    memory = device->device.allocateMemory(allocInfo);

    // the fourth parameter is the offset within the region of memory. Since
    // this memory is allocated specifically for this the vertex buffer, the
    // offset is simply 0. If the offset is non-zero, then it is required to be
    // divisible by memRequirements.alignment.
    buffer.bindMemory(*memory, 0);
}

void Buffer::createBufferWithMemory(const LogicalDevice *device,
                                    const vk::DeviceSize size,
                                    const vk::BufferUsageFlags usage,
                                    const vk::MemoryPropertyFlags properties,
                                    vk::raii::Buffer &buffer, vk::raii::DeviceMemory &memory) {
    createBuffer(device, size, usage, buffer);

    // The buffer has been created, but it doesn't actually have any memory
    // assigned to it yet.
    allocateMemory(device, properties, buffer, memory);
}

/*
TODO:

https://developer.nvidia.com/vulkan-memory-management

It should be noted that in a real world application, you're not supposed to
actually call vkAllocateMemory for every individual buffer. The maximum number
of simultaneous memory allocations is limited by the maxMemoryAllocationCount
physical device limit, which may be as low as 4096 even on high end hardware
like an NVIDIA GTX 1080. The right way to allocate memory for a large number of
objects at the same time is to create a custom allocator that splits up a single
allocation among many different objects by using the offset parameters that
we've seen in many functions.

You can either implement such an allocator yourself, or use the
VulkanMemoryAllocator library provided by the GPUOpen initiative. However, for
this tutorial it's okay to use a separate allocation for every resource, because
we won't come close to hitting any of these limits for now.


*/

void Buffer::copyFromCPU(const void *cpuData) {
    // Unfortunately the driver may not immediately copy the data into the
    // buffer memory, for example because of caching. It is also possible that
    // writes to the buffer are not visible in the mapped memory yet. There are
    // two ways to deal with that problem:
    //    - Use a memory heap that is host coherent, indicated with
    //      vk::MemoryPropertyFlagBits::eHostCoherent.
    //    - Call vkFlushMappedMemoryRanges after writing to the mapped memory,
    //      and call vkInvalidateMappedMemoryRanges before reading from the
    //      mapped memory (faster!).

    void *data = memory.mapMemory(0, size, {});
    memcpy(data, cpuData, (size_t)size);
    memory.unmapMemory();
}

inline void copyBuffer(vk::Device device, vk::CommandPool commandPool,
                       vk::Buffer srcBuffer, vk::Buffer dstBuffer,
                       vk::DeviceSize size, vk::Queue transferQueue) {
    // Create temporary command buffer and submit the copy operation

    SingleTimeCommandManager manager(commandPool, device, transferQueue);
    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    manager.commandBuffers[0].copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
   
}

void StagedBuffer::copyFromCPU(const void *cpuData, vk::CommandPool commandPool) {

    Buffer buf(device, size, vk::BufferUsageFlagBits::eTransferSrc,
               vk::MemoryPropertyFlagBits::eHostVisible |
                   vk::MemoryPropertyFlagBits::eHostCoherent);

    buf.copyFromCPU(cpuData);

    copyBuffer(device->handle(), commandPool, *buf.buffer, *buffer, size,
               *device->transferQueue);
}
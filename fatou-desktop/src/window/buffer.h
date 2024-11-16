#pragma once

#include <array>
#include "LogicalDevice.h"
#include <glm/glm.hpp>

inline vk::Format findSupportedFormat(const PhysicalDevice *physicalDevice,
                                      const std::vector<vk::Format> &candidates,
                                      vk::ImageTiling tiling,
                                      vk::FormatFeatureFlags features) {
    for (vk::Format format : candidates) {
        const vk::FormatProperties props =
            physicalDevice->device.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == vk::ImageTiling::eOptimal &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

inline vk::Format getSupportedDepthFormat(const PhysicalDevice *device) {
    // Since all depth formats may be optional, we need to find a suitable depth
    // format to use Start with the highest precision packed format
    std::vector<vk::Format> depthFormats = {
        vk::Format::eD32SfloatS8Uint, vk::Format::eD32Sfloat,
        vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint,
        vk::Format::eD16Unorm};
    return findSupportedFormat(
        device, depthFormats, vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

inline vk::Format getSupportedStencilFormat(const PhysicalDevice *device) {
    // focus on Stencil buffer only (but still have a depth buffer)

    std::vector<vk::Format> depthFormats = {
        // vk::Format::eS8Uint,
        vk::Format::eD16UnormS8Uint,
        vk::Format::eD32SfloatS8Uint,
        vk::Format::eD24UnormS8Uint,
    };
    return findSupportedFormat(
        device, depthFormats, vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

inline uint32_t findMemoryType(const LogicalDevice *device,
                               const uint32_t typeFilter,
                               const vk::MemoryPropertyFlags properties) {
    // Memory heaps are distinct memory resources like dedicated VRAM and swap
    // space in RAM for when VRAM runs out. The different types of memory exist
    // within these heaps. Right now we'll only concern ourselves with the type
    // of memory and not the heap it comes from, but you can imagine that this
    // can affect performance.
    vk::PhysicalDeviceMemoryProperties memProperties =
        device->physical->device.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    throw runtime_error("failed to find suitable memory type!");
}
class SingleTimeCommandManager : private boost::noncopyable {
    // TODO: All of the helper functions that submit commands so far have been
    // set up to execute synchronously by waiting for the queue to become idle.
    // For practical applications it is recommended to combine these operations
    // in a single command buffer and execute them asynchronously for higher
    // throughput, especially the transitions and copy in the createTextureImage
    // function. Try to experiment with this by creating a setupCommandBuffer
    // that the helper functions record commands into, and add a
    // flushSetupCommands to execute the commands that have been recorded so
    // far. It's best to do this after the texture mapping works to check if the
    // texture resources are still set up correctly.

  public:
    SingleTimeCommandManager(vk::CommandPool commandPool, vk::Device device,
                             vk::Queue queue)
        : device(device), commandPool(commandPool), queue(queue) {
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        commandBuffers = device.allocateCommandBuffers(allocInfo);

        vk::CommandBufferBeginInfo beginInfo{};
        // VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;
        // We're only going to use the command buffer once and wait with
        // returning from the function until the copy operation has finished
        // executing.
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        commandBuffers[0].begin(beginInfo);
    }
    ~SingleTimeCommandManager() {

        commandBuffers[0].end();

        vk::SubmitInfo submitInfo{};
        submitInfo.sType = vk::StructureType::eSubmitInfo;
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = commandBuffers.data();

        queue.submit(submitInfo, {});

        // Unlike the draw commands, there are no events we need to wait on this
        // time. We just want to execute the transfer on the buffers
        // immediately. There are again two possible ways to wait on this
        // transfer to complete. We could use a fence and wait with
        // vkWaitForFences, or simply wait for the transfer queue to become idle
        // with vkQueueWaitIdle. A fence would allow you to schedule multiple
        // transfers simultaneously and wait for all of them complete, instead
        // of executing one at a time. That may give the driver more
        // opportunities to optimize.
        queue.waitIdle();

        // vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        // device.freeCommandBuffers(commandPool, commandBuffers);
    }

  private:
    const vk::Device device;
    const vk::CommandPool commandPool;
    const vk::Queue queue;

  public:
    vector<vk::CommandBuffer> commandBuffers;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 3>
    getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 3>
            attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

struct Vertex2 {
    glm::vec2 pos;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription() {
        vk::VertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex2);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;

        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
        std::array<vk::VertexInputAttributeDescription, 2>
            attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[0].offset = offsetof(Vertex2, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
        attributeDescriptions[1].offset = offsetof(Vertex2, texCoord);

        return attributeDescriptions;
    }
};

class Buffer : private boost::noncopyable {
  public:
    Buffer(shared_ptr<LogicalDevice> device, vk::DeviceSize size,
           vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
        : device(device), size(size), buffer(nullptr), memory(nullptr) {
        createBufferWithMemory(device.get(), size, usage, properties, buffer,
                               memory);
    }

    ~Buffer() {
        //  vkDestroyBuffer(device->handle(), buffer, nullptr);
        //  vkFreeMemory(device->handle(), memory, nullptr);
    }

    void copyFromCPU(const void *data);

    vk::Buffer handle() const { return *buffer; }

  protected:
    static void createBufferWithMemory(const LogicalDevice *device,
                                       const vk::DeviceSize size,
                                       const vk::BufferUsageFlags usage,
                                       const vk::MemoryPropertyFlags properties,
                                       vk::raii::Buffer &buffer,
                                       vk::raii::DeviceMemory &memory);

  protected:
    shared_ptr<LogicalDevice> device;
    const vk::DeviceSize size;

    vk::raii::Buffer buffer;
    vk::raii::DeviceMemory memory;

    friend class StagedBuffer;
};

// https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer
// TODO: The previous chapter already mentioned that you should allocate
// multiple resources like buffers from a single memory allocation, but in fact
// you should go a step further. Driver developers recommend that you also store
// multiple buffers, like the vertex and index buffer, into a single vk::Buffer
// and use offsets in commands like vkCmdBindVertexBuffers. The advantage is
// that your data is more cache friendly in that case, because it's closer
// together. It is even possible to reuse the same chunk of memory for multiple
// resources if they are not used during the same render operations, provided
// that their data is refreshed, of course. This is known as aliasing and some
// Vulkan functions have explicit flags to specify that you want to do this.

class StagedBuffer : protected Buffer {
  public:
    StagedBuffer(shared_ptr<LogicalDevice> device, vk::DeviceSize size,
                 vk::BufferUsageFlags usage)
        : Buffer(device, size, vk::BufferUsageFlagBits::eTransferDst | usage,
                 vk::MemoryPropertyFlagBits::eDeviceLocal) {}

    void copyFromCPU(const void *data, vk::CommandPool commandPool);
};

template <typename T> class VertexBuffer : protected StagedBuffer {
  public:
    VertexBuffer(shared_ptr<LogicalDevice> device, const vector<T> &data,
                 vk::CommandPool commandPool)
        : StagedBuffer(device, data.size() * sizeof(T),
                       vk::BufferUsageFlagBits::eVertexBuffer) {
        copyFromCPU(data.data(), commandPool);
    }

    void bind(const vk::CommandBuffer commandBuffer) {
        std::array<vk::Buffer, 1> vertexBuffers = {*buffer};
        std::array<vk::DeviceSize, 1> offsets = {0};
        commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
    }
};

class IndexBuffer : protected StagedBuffer {
  public:
    IndexBuffer(shared_ptr<LogicalDevice> device, const vector<uint16_t> &data,
                vk::CommandPool commandPool)
        : StagedBuffer(device, data.size() * sizeof(uint16_t),
                       vk::BufferUsageFlagBits::eIndexBuffer) {
        copyFromCPU(data.data(), commandPool);
    }

    void bind(const vk::CommandBuffer commandBuffer) {
        // The difference to vertex Buffers is that you can only have a single
        // index buffer. It's unfortunately not possible to use different
        // indices for each vertex attribute, so we do still have to completely
        // duplicate vertex data even if just one attribute varies.
        commandBuffer.bindIndexBuffer(*buffer, 0, vk::IndexType::eUint16);
    }
};
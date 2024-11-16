#pragma once

#include "logicalDevice.h"
#include "buffer.h"

#define GLM_FORCE_RADIANS
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

struct UniformBufferObject {
    // UBOs must be aligned!
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct UniformBufferObject2 {
    // UBOs must be aligned! (and not over-aligned!)
    // float = 4 bytes
    // vec2 = 8 bytes
    // vec3, vec4, mat4 = 16 bytes
    /* alignas(8) glm::vec2 pos;
     alignas(4) float zoom;*/
    alignas(16) glm::dvec2 pos;
    alignas(8) double zoom;
    alignas(4) int iter;
    alignas(4) float iGamma;
    alignas(4) float play;
    alignas(4) float shift;
    alignas(4) float contrast;
    alignas(4) float phase;
    alignas(4) float radius;
    alignas(4) float smoothing;
};

class DescriptorSetLayout {
  public:
    DescriptorSetLayout(shared_ptr<LogicalDevice> device) : device(device) {
        createDescriptorSetLayout();
    }
    void createDescriptorSetLayout();

    ~DescriptorSetLayout() {}

    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

  private:
    const shared_ptr<LogicalDevice> device;
};

class DescriptorSetLayoutVertexOnly {
  public:
    DescriptorSetLayoutVertexOnly(shared_ptr<LogicalDevice> device)
        : device(device) {
        createDescriptorSetLayout();
    }
    void createDescriptorSetLayout();

    ~DescriptorSetLayoutVertexOnly() {}

    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

  private:
    const shared_ptr<LogicalDevice> device;
};

class MandelDescriptorSetLayout {
  public:
    MandelDescriptorSetLayout(shared_ptr<LogicalDevice> device)
        : device(device) {
        createDescriptorSetLayout();
    }
    void createDescriptorSetLayout();

    ~MandelDescriptorSetLayout() {
        //  vkDestroyDescriptorSetLayout(device->handle(), descriptorSetLayout,
        //  nullptr);
    }

    vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;

  private:
    const shared_ptr<LogicalDevice> device;
};

class DescriptorPool {
    // Unlike vertex and index buffers, descriptor sets are not unique to
    // graphics pipelines.

    // it is actually possible to bind multiple descriptor sets simultaneously.
    // You can use this feature to put descriptors that vary per-object and
    // descriptors that are shared into separate descriptor sets. In that case
    // you avoid rebinding most of the descriptors across draw calls which is
    // potentially more efficient.

  public:
    DescriptorPool(size_t MAX_FRAMES_IN_FLIGHT,
                   shared_ptr<LogicalDevice> device,
                   vk::DescriptorSetLayout descriptorSetLayout,
                   vk::ImageView textureImageView, vk::Sampler textureSampler)
        : MAX_FRAMES_IN_FLIGHT(MAX_FRAMES_IN_FLIGHT), device(device),
          descriptorPool(0) {
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets(descriptorSetLayout, textureImageView,
                             textureSampler);
    };

    ~DescriptorPool() {
        // You don't need to explicitly clean up descriptor sets, because they
        // will be automatically freed when the descriptor pool is destroyed.
        // vkDestroyDescriptorPool(device->handle(), descriptorPool, nullptr);
    }

    void update(uint32_t currentImage, const Extent2D extent,
                const UniformBufferObject &ubo);

    void bind(vk::CommandBuffer commandBuffer, uint32_t currentFrame,
              vk::PipelineLayout pipelineLayout) {
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, pipelineLayout,
            0,                                // index of first descriptor set
            1,                                // number of sets to bind
            &(*descriptorSets[currentFrame]), // array of sets to bind
            0,                                // for dynamic descriptors
            nullptr);
    }

  private:
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets(vk::DescriptorSetLayout descriptorSetLayout,
                              vk::ImageView textureImageView,
                              vk::Sampler textureSampler);

  private:
    const size_t MAX_FRAMES_IN_FLIGHT;
    const shared_ptr<LogicalDevice> device;

    vk::raii::DescriptorPool descriptorPool;
    vector<vk::raii::DescriptorSet> descriptorSets;

    vector<shared_ptr<Buffer>> uniformBuffers;
};

template <class UBO1, class UBO2> class DefaultDescriptorPool {
  public:
    DefaultDescriptorPool(shared_ptr<LogicalDevice> device,
                          vk::DescriptorSetLayout descriptorSetLayout)
        : device(device), descriptorPool(0) {
        createUniformBuffer();
        createDescriptorPool();
        createDescriptorSet(descriptorSetLayout);
    };

    ~DefaultDescriptorPool() {
        // You don't need to explicitly clean up descriptor sets, because they
        // will be automatically freed when the descriptor pool is destroyed.
        //  vkDestroyDescriptorPool(device->handle(), descriptorPool,
        //  nullptr);
    }

    void updateVertex(const UBO1 &ubo) {
        // PLEASE NOTE: Using a UBO this way is not the most efficient way to
        // pass frequently changing values to the shader. A more efficient way
        // to pass a small buffer of data to shaders are push constants.

        uniformBuffer->copyFromCPU(&ubo);
    }

    void updateFragment(const UBO2 &ubo2) {
        // PLEASE NOTE: Using a UBO this way is not the most efficient way to
        // pass frequently changing values to the shader. A more efficient way
        // to pass a small buffer of data to shaders are push constants.

        uniformBuffer2->copyFromCPU(&ubo2);
    }

    void bind(vk::CommandBuffer commandBuffer,
              vk::PipelineLayout pipelineLayout) {
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, pipelineLayout,
            0,                    // index of first descriptor set
            1,                    // number of sets to bind
            &(*descriptorSet[0]), // array of sets to bind
            0,                    // for dynamic descriptors
            nullptr);
    }

  private:
    void createUniformBuffer() {
        vk::DeviceSize bufferSize = sizeof(UBO1);

        uniformBuffer = make_shared<Buffer>(
            device, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent);

        vk::DeviceSize bufferSize2 = sizeof(UBO2);

        uniformBuffer2 = make_shared<Buffer>(
            device, bufferSize2, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent);
    }

    void createDescriptorPool() {

        std::array<vk::DescriptorPoolSize, 1> poolSizes{};
        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        poolSizes[0].descriptorCount = 2;

        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        // Maximum number of descriptor sets that may be allocated
        poolInfo.maxSets = 1;

        // The structure has an optional flag similar to command pools that
        // determines if individual descriptor sets can be freed or not:
        // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
        poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

        descriptorPool = device->device.createDescriptorPool(poolInfo);
    }

    void createDescriptorSet(vk::DescriptorSetLayout descriptorSetLayout) {
        std::vector<vk::DescriptorSetLayout> layouts(1, descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
        allocInfo.descriptorPool = *descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts.data();

        descriptorSet = device->device.allocateDescriptorSets(allocInfo);

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
        size_t i = 0;

        ////////

        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer->handle();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UBO1);

        descriptorWrites[i].sType = vk::StructureType::eWriteDescriptorSet;
        descriptorWrites[i].dstSet = *descriptorSet[0];
        // We gave our uniform buffer binding index 0
        descriptorWrites[i].dstBinding = 0;
        // Remember that descriptors can be arrays, so we also need to specify
        // the first index in the array that we want to update.
        descriptorWrites[i].dstArrayElement = 0;

        descriptorWrites[i].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[i].descriptorCount = 1;

        descriptorWrites[i].pBufferInfo = &bufferInfo;
        descriptorWrites[i].pImageInfo = nullptr;       // Optional
        descriptorWrites[i].pTexelBufferView = nullptr; // Optional

        ////////

        vk::DescriptorBufferInfo bufferInfo2{};
        bufferInfo2.buffer = uniformBuffer2->handle();
        bufferInfo2.offset = 0;
        bufferInfo2.range = sizeof(UBO2);

        i = 1;
        descriptorWrites[i].sType = vk::StructureType::eWriteDescriptorSet;
        descriptorWrites[i].dstSet = *descriptorSet[0];
        descriptorWrites[i].dstBinding = 1;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pBufferInfo = &bufferInfo2;
        descriptorWrites[i].pImageInfo = nullptr;       // Optional
        descriptorWrites[i].pTexelBufferView = nullptr; // Optional

        device->device.updateDescriptorSets(descriptorWrites, {});
    }

  private:
    const shared_ptr<LogicalDevice> device;

    vk::raii::DescriptorPool descriptorPool;
    vector<vk::raii::DescriptorSet> descriptorSet;

    shared_ptr<Buffer> uniformBuffer;
    shared_ptr<Buffer> uniformBuffer2;
};

template <class UBO1, class UBO2> class DefaultDescriptorPoolVertex {
  public:
    DefaultDescriptorPoolVertex(shared_ptr<LogicalDevice> device,
                                vk::DescriptorSetLayout descriptorSetLayout)
        : device(device), descriptorPool(0) {
        createUniformBuffer();
        createDescriptorPool();
        createDescriptorSet(descriptorSetLayout);
    };

    ~DefaultDescriptorPoolVertex() {
        // You don't need to explicitly clean up descriptor sets, because they
        // will be automatically freed when the descriptor pool is destroyed.
        //  vkDestroyDescriptorPool(device->handle(), descriptorPool,
        //  nullptr);
    }

    void updateVertex(const UBO1 &ubo) {
        // PLEASE NOTE: Using a UBO this way is not the most efficient way to
        // pass frequently changing values to the shader. A more efficient way
        // to pass a small buffer of data to shaders are push constants.

        uniformBuffer->copyFromCPU(&ubo);
    }

    // TODO: remove
    void updateFragment(const UBO2 &ubo) {}

    void bind(vk::CommandBuffer commandBuffer,
              vk::PipelineLayout pipelineLayout) {
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics, pipelineLayout,
            0,                    // index of first descriptor set
            1,                    // number of sets to bind
            &(*descriptorSet[0]), // array of sets to bind
            0,                    // for dynamic descriptors
            nullptr);
    }

  private:
    void createUniformBuffer() {
        vk::DeviceSize bufferSize = sizeof(UBO1);

        uniformBuffer = make_shared<Buffer>(
            device, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent);
    }

    void createDescriptorPool() {
        std::array<vk::DescriptorPoolSize, 1> poolSizes{};
        poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
        poolSizes[0].descriptorCount = 1;

        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        // Maximum number of descriptor sets that may be allocated
        poolInfo.maxSets = 1;

        // The structure has an optional flag similar to command pools that
        // determines if individual descriptor sets can be freed or not:
        // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
        poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

        descriptorPool = device->device.createDescriptorPool(poolInfo);
    }

    void createDescriptorSet(vk::DescriptorSetLayout descriptorSetLayout) {
        std::vector<vk::DescriptorSetLayout> layouts(1, descriptorSetLayout);
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
        allocInfo.descriptorPool = *descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts.data();

        descriptorSet = device->device.allocateDescriptorSets(allocInfo);

        std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};
        size_t i = 0;

        ////////

        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer->handle();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UBO1);

        descriptorWrites[i].sType = vk::StructureType::eWriteDescriptorSet;
        descriptorWrites[i].dstSet = *descriptorSet[0];
        // We gave our uniform buffer binding index 0
        descriptorWrites[i].dstBinding = 0;
        // Remember that descriptors can be arrays, so we also need to specify
        // the first index in the array that we want to update.
        descriptorWrites[i].dstArrayElement = 0;

        descriptorWrites[i].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[i].descriptorCount = 1;

        descriptorWrites[i].pBufferInfo = &bufferInfo;
        descriptorWrites[i].pImageInfo = nullptr;       // Optional
        descriptorWrites[i].pTexelBufferView = nullptr; // Optional

        ////////

        device->device.updateDescriptorSets(descriptorWrites, {});
    }

  private:
    const shared_ptr<LogicalDevice> device;

    vk::raii::DescriptorPool descriptorPool;
    vector<vk::raii::DescriptorSet> descriptorSet;

    shared_ptr<Buffer> uniformBuffer;
};

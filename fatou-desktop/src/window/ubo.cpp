

#include <chrono>

#include "ubo.h"

void DescriptorSetLayout::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;

    //  It is possible for the shader variable to represent an array of
    //  uniform buffer objects, and descriptorCount specifies the number of
    //  values in the array. This could be used to specify a transformation
    //  for each of the bones in a skeleton for skeletal animation, for
    //  example.
    uboLayoutBinding.descriptorCount = 1;

    // e.g. VK_SHADER_STAGE_ALL_GRAPHICS
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    // for images
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    ///////////

    vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType =
        vk::DescriptorType::eCombinedImageSampler;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    // Make sure to set the stageFlags to indicate that we intend to use the
    // combined image sampler descriptor in the fragment shader. It is possible
    // to use texture sampling in the vertex shader, for example to dynamically
    // deform a grid of vertices by a heightmap.
    samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    /////////

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
        uboLayoutBinding, samplerLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = device->device.createDescriptorSetLayout(layoutInfo);
}

void DescriptorSetLayoutVertexOnly::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;

    //  It is possible for the shader variable to represent an array of
    //  uniform buffer objects, and descriptorCount specifies the number of
    //  values in the array. This could be used to specify a transformation
    //  for each of the bones in a skeleton for skeletal animation, for
    //  example.
    uboLayoutBinding.descriptorCount = 1;

    // e.g. VK_SHADER_STAGE_ALL_GRAPHICS
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    // for images
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    ///////////

    std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {uboLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = device->device.createDescriptorSetLayout(layoutInfo);
}

void MandelDescriptorSetLayout::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;

    //  It is possible for the shader variable to represent an array of
    //  uniform buffer objects, and descriptorCount specifies the number of
    //  values in the array. This could be used to specify a transformation
    //  for each of the bones in a skeleton for skeletal animation, for
    //  example.
    uboLayoutBinding.descriptorCount = 1;

    // e.g. VK_SHADER_STAGE_ALL_GRAPHICS
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    // for images
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    ///////////

    vk::DescriptorSetLayoutBinding fragmentUboLayoutBinding{};
    fragmentUboLayoutBinding.binding = 1;
    fragmentUboLayoutBinding.descriptorCount = 1;
    fragmentUboLayoutBinding.descriptorType =
        vk::DescriptorType::eUniformBuffer;
    fragmentUboLayoutBinding.pImmutableSamplers = nullptr;
    fragmentUboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    /////////

    std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {
        uboLayoutBinding, fragmentUboLayoutBinding};
    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    descriptorSetLayout = device->device.createDescriptorSetLayout(layoutInfo);
}

void DescriptorPool::update(uint32_t currentImage, const Extent2D extent,
                            const UniformBufferObject &ubo) {
    // PLEASE NOTE: Using a UBO this way is not the most efficient way to pass
    // frequently changing values to the shader. A more efficient way to pass a
    // small buffer of data to shaders are push constants.

    uniformBuffers[currentImage]->copyFromCPU(&ubo);
}

void DescriptorPool::createUniformBuffers() {
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

    // uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        uniformBuffers.push_back(make_shared<Buffer>(
            device, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible |
                vk::MemoryPropertyFlagBits::eHostCoherent));
    }
}

void DescriptorPool::createDescriptorPool() {

    // Inadequate descriptor pools are a good example of a problem that the
    // validation layers will not catch: As of Vulkan 1.1,
    // vkAllocateDescriptorSets may fail with the error code
    // VK_ERROR_POOL_OUT_OF_MEMORY if the pool is not sufficiently large, but
    // the driver may also try to solve the problem internally. This means that
    // sometimes (depending on hardware, pool size and allocation size) the
    // driver will let us get away with an allocation that exceeds the limits of
    // our descriptor pool. Other times, vkAllocateDescriptorSets will fail and
    // return VK_ERROR_POOL_OUT_OF_MEMORY. This can be particularly frustrating
    // if the allocation succeeds on some machines, but fails on others.

    std::array<vk::DescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    // Maximum number of descriptor sets that may be allocated
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    // The structure has an optional flag similar to command pools that
    // determines if individual descriptor sets can be freed or not:
    // VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    descriptorPool = device->device.createDescriptorPool(poolInfo);
}

void DescriptorPool::createDescriptorSets(
    vk::DescriptorSetLayout descriptorSetLayout, vk::ImageView textureImageView,
    vk::Sampler textureSampler) {
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                                 descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
    allocInfo.descriptorPool = *descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets = device->device.allocateDescriptorSets(allocInfo);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vk::DescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i]->handle();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        imageInfo.imageView = textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = vk::StructureType::eWriteDescriptorSet;
        descriptorWrites[0].dstSet = *descriptorSets[i];
        // We gave our uniform buffer binding index 0
        descriptorWrites[0].dstBinding = 0;
        // Remember that descriptors can be arrays, so we also need to specify
        // the first index in the array that we want to update.
        descriptorWrites[0].dstArrayElement = 0;

        descriptorWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
        descriptorWrites[0].descriptorCount = 1;

        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = nullptr;       // Optional
        descriptorWrites[0].pTexelBufferView = nullptr; // Optional

        descriptorWrites[1].sType = vk::StructureType::eWriteDescriptorSet;
        descriptorWrites[1].dstSet = *descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            vk::DescriptorType::eCombinedImageSampler;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = nullptr; // Optional
        descriptorWrites[1].pImageInfo = &imageInfo;
        descriptorWrites[1].pTexelBufferView = nullptr; // Optional

        device->device.updateDescriptorSets(descriptorWrites, {});
    }
}

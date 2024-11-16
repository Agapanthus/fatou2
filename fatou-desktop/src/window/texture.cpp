#include "texture.h"

#include "webp/decode.h"
#include "../shaderc/database.h"
#include <fstream>

shared_ptr<Texture> Texture::fromFile(shared_ptr<LogicalDevice> device,
                                      const string &path,
                                      vk::CommandPool commandPool,
                                      vk::Queue transferQueue) {
    int w;
    int h;
    uint8_t *data = loadFile(path, w, h);
    if (!data) {
        throw runtime_error("image load failed");
    }
    auto t =
        make_shared<Texture>(device, data, w, h, commandPool, transferQueue);
    WebPFree(data);
    return t;
}

void createImage(const LogicalDevice *device, int w, int h, vk::Format format,
                 vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                 vk::MemoryPropertyFlags properties, vk::raii::Image &image,
                 vk::raii::DeviceMemory &imageMemory) {
    vk::ImageCreateInfo imageInfo{};
    imageInfo.sType = vk::StructureType::eImageCreateInfo;

    // It is possible to create 1D, 2D and 3D images. One dimensional images
    // can be used to store an array of data or gradient, two dimensional
    // images are mainly used for textures, and three dimensional images can
    // be used to store voxel volumes, for example.
    imageInfo.imageType = vk::ImageType::e2D;

    imageInfo.extent.width = static_cast<uint32_t>(w);
    imageInfo.extent.height = static_cast<uint32_t>(h);
    imageInfo.extent.depth = 1;

    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    // TODO: SRGB might be not available; have fallbacks!
    imageInfo.format = format; // vk::Format::eR8G8B8A8Srgb;

    // The tiling field can have one of two values:
    // -  vk::ImageTiling::eLinear: Texels are laid out in row-major order like
    //   our pixels array
    // -  vk::ImageTiling::eOptimal: Texels are laid out in an implementation
    //   defined order for optimal access
    imageInfo.tiling = tiling; //  vk::ImageTiling::eOptimal;

    // Can also be vk::ImageLayout::_PREINITIALIZED
    // One example, however, would be if you wanted to use an image as a
    // staging image in combination with the  vk::ImageTiling::eLinear layout.
    // In that case, you'd want to upload the texel data to it and then
    // transition the image to be a transfer source without losing the data.
    imageInfo.initialLayout = vk::ImageLayout::eUndefined;

    imageInfo.usage = usage;

    // not shared between queue families
    imageInfo.sharingMode = vk::SharingMode::eExclusive;

    // The samples flag is related to multisampling. This is only relevant
    // for images that will be used as attachments, so stick to one sample.
    imageInfo.samples = vk::SampleCountFlagBits::e1;

    // There are some optional flags for images that are related to sparse
    // images. Sparse images are images where only certain regions are
    // actually backed by memory. If you were using a 3D texture for a voxel
    // terrain, for example, then you could use this to avoid allocating
    // memory to store large volumes of "air" values.
    imageInfo.flags = {}; // Optional

    image = device->device.createImage(imageInfo);

    vk::MemoryRequirements memRequirements = image.getMemoryRequirements();

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.sType = vk::StructureType::eMemoryAllocateInfo;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(device, memRequirements.memoryTypeBits, properties);

    imageMemory = device->device.allocateMemory(allocInfo);

    // TODO: Why not device?
    // vk::BindImageMemoryInfo(*image, *imageMemory, 0);
    image.bindMemory(*imageMemory, 0);
    // vkBindImageMemory(device->handle(), image, imageMemory, 0);
}

void copyBufferToImage(vk::Device device, vk::CommandPool commandPool,
                       vk::Queue transferQueue, vk::Buffer buffer,
                       vk::Image image, uint32_t width, uint32_t height,
                       vk::ImageLayout layout,
                       vk::ImageAspectFlags aspectMask) {
    SingleTimeCommandManager manager(commandPool, device, transferQueue);

    // Just like with buffer copies, you need to specify which part of the
    // buffer is going to be copied to which part of the image.
    vk::BufferImageCopy region{};
    region.bufferOffset = 0;

    // i.e., add some padding between rows and columns
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = aspectMask;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D({0, 0, 0});
    region.imageExtent = vk::Extent3D({width, height, 1});

    assert(layout == vk::ImageLayout::eTransferDstOptimal);
    manager.commandBuffers[0].copyBufferToImage(buffer, image,
                                                layout, // current layout
                                                1, &region);
}

void transitionImageLayout(vk::Device device, vk::CommandPool commandPool,
                           vk::Queue transferQueue, vk::Image image,
                           vk::Format format, vk::ImageLayout oldLayout,
                           vk::ImageLayout newLayout,
                           vk::ImageAspectFlags aspectMask) {
    SingleTimeCommandManager manager(commandPool, device, transferQueue);
    // TODO: Create a semaphore-based version instead of a barrier-based one!
    // (faster?)

    // One of the most common ways to perform layout transitions is using an
    // image memory barrier. A pipeline barrier like that is generally used to
    // synchronize access to resources, like ensuring that a write to a buffer
    // completes before reading from it, but it can also be used to transition
    // image layouts and transfer queue family ownership when
    // vk::SharingMode::eExclusive is used.

    vk::ImageMemoryBarrier barrier{};
    barrier.sType = vk::StructureType::eImageMemoryBarrier;

    //  It is possible to use vk::ImageLayout::_UNDEFINED as oldLayout if you
    //  don't care about the existing contents of the image.
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    // If you are using the barrier to transfer queue family ownership, then
    // these two fields should be the indices of the queue families. They must
    // be set to VK_QUEUE_FAMILY_IGNORED if you don't want to do this
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    // you must specify which types of operations that involve the resource must
    // happen before the barrier, and which operations that involve the resource
    // must wait on the barrier. We need to do that despite already using
    // vkQueueWaitIdle to manually synchronize.
    // There is actually a special type of image layout that supports all
    // operations, vk::ImageLayout::_GENERAL. The problem with it, of course, is
    // that it doesn't necessarily offer the best performance for any operation.
    // It is required for some special cases, like using an image as both input
    // and output, or for reading an image after it has left the preinitialized
    // layout.
    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;
    /*if (oldLayout == vk::ImageLayout::eUndefined &&
        newLayout == vk::ImageLayout::eTransferDstOptimal) {

        // Since the writes don't have to wait on anything, you may specify an
        // empty access mask and the earliest possible pipeline stage
        // vk::PipelineStageFlagBits::eTopOfPipe for the pre-barrier operations.
        barrier.srcAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

        // vk::PipelineStageFlagBits::eTransfer is not a real stage within the
        // graphics and compute pipelines. It is more of a pseudo-stage where
        // transfers happen.
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

    } else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

    } else if (oldLayout == vk::ImageLayout::eUndefined &&
               newLayout == vk::ImageLayout::eGeneral) {
        // TODO: I have no idea. Is this correct?
        barrier.srcAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
               newLayout == vk::ImageLayout::eGeneral) {
        // TODO: I have no idea. Is this correct?
        barrier.srcAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
               newLayout == vk::ImageLayout::eTransferDstOptimal) {
        // TODO: I have no idea. Is this correct?
        barrier.srcAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal &&
               newLayout == vk::ImageLayout::eTransferSrcOptimal) {
        // TODO: I have no idea. Is this correct?
        barrier.srcAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal &&
               newLayout == vk::ImageLayout::eGeneral) {
        // TODO: I have no idea. Is this correct?
        barrier.srcAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
               newLayout == vk::ImageLayout::eGeneral) {
        // TODO: I have no idea. Is this correct?
        barrier.srcAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;

    } else if (oldLayout == vk::ImageLayout::eGeneral &&
               newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        // TODO: I have no idea. Is this correct?
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;

        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

    } else {
        throw invalid_argument("unsupported layout transition!");
    }*/

    // The only arguments we're not sure about are srcFlags and dstFlags. We
    // know we want our execution / memory dependencies to be staged at the top
    // of the command buffer. So, we'll use VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT to
    // notify Vulkan of our intentions. You can find more information on
    // pipeline state flags like VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT here.
    // TODO: Use better stage names!
    sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTopOfPipe;

    // https://harrylovescode.gitbooks.io/vulkan-api/content/chap07/chap07.html
    switch (oldLayout) {
    case vk::ImageLayout::eUndefined:
        barrier.srcAccessMask = {};
        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe; // TODO
        break;
    case vk::ImageLayout::ePreinitialized:
        barrier.srcAccessMask =
            vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        break;
    /*case vk::ImageLayout::eColorAttachmentOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        break;
    case vk::ImageLayout::eStencilAttachmentOptimal:
        barrier.srcAccessMask =
            vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        break;*/
    case vk::ImageLayout::eTransferSrcOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer; // TODO
        break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        sourceStage = vk::PipelineStageFlagBits::eFragmentShader; // TODO
        break;
    case vk::ImageLayout::eTransferDstOptimal:
        // TODO: Correct?
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        sourceStage = vk::PipelineStageFlagBits::eTransfer; // TODO
        break;

    default:
        throw invalid_argument("unsupported layout transition!");
    }

    switch (newLayout) {
    case vk::ImageLayout::eTransferDstOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
        break;
    case vk::ImageLayout::eTransferSrcOptimal:
        // TODO
        // barrier.srcAccessMask |= vk::AccessFlagBits::eTransferRead;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;      // TODO
        destinationStage = vk::PipelineStageFlagBits::eTransfer; // TODO
        break;
    case vk::ImageLayout::eColorAttachmentOptimal:
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        break;
    case vk::ImageLayout::eDepthStencilAttachmentOptimal:
        barrier.dstAccessMask |=
            vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
        break;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        // TODO
        barrier.srcAccessMask =
            /*vk::AccessFlagBits::eHostWrite |*/ vk::AccessFlagBits::
                eTransferWrite |
            vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eTransfer; // TODO

        break;
    default:
        throw invalid_argument("unsupported layout transition!");
    }

    // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap7.html#synchronization-pipeline-stages
    // For example, if you're going to read from a uniform after the barrier,
    // you would specify a usage of VK_ACCESS_UNIFORM_READ_BIT and the earliest
    // shader that will read from the uniform as pipeline stage, for example
    // vk::PipelineStageFlagBits::eFragmentShader. It would not make sense to
    // specify a non-shader pipeline stage for this type of usage and the
    // validation layers will warn you when you specify a pipeline stage that
    // does not match the type of usage.
    manager.commandBuffers[0].pipelineBarrier(
        sourceStage,      // stage before
        destinationStage, // stage after, i.e., wait)
        {},               // {} or VK_DEPENDENCY_BY_REGION_BIT
        0, nullptr,       // memory barriers
        0, nullptr,       // buffer barriers
        1, &barrier       // image memory barriers
    );
}

void Texture::createTextureImage(const uint8_t *data,
                                 vk::CommandPool commandPool,
                                 vk::Queue transferQueue) {

    Buffer buf(device, w * h * 4, vk::BufferUsageFlagBits::eTransferSrc,
               vk::MemoryPropertyFlagBits::eHostVisible |
                   vk::MemoryPropertyFlagBits::eHostCoherent);
    buf.copyFromCPU(data);

    createImage(device.get(), w, h, vk::Format::eR8G8B8A8Srgb,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransferDst |
                    vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage,
                textureImageMemory);

    // Transition: undefined → transfer destination:
    transitionImageLayout(*device->device, commandPool, transferQueue,
                          *textureImage, vk::Format::eR8G8B8A8Srgb,
                          vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eTransferDstOptimal, aspectMask);

    // Execute the buffer to image copy operation
    copyBufferToImage(*device->device, commandPool, transferQueue, buf.handle(),
                      *textureImage, static_cast<uint32_t>(w),
                      static_cast<uint32_t>(h),
                      vk::ImageLayout::eTransferDstOptimal, aspectMask);

    // Transition: transfer destination → shader reading
    transitionImageLayout(*device->device, commandPool, transferQueue,
                          *textureImage, vk::Format::eR8G8B8A8Srgb,
                          vk::ImageLayout::eTransferDstOptimal,
                          vk::ImageLayout::eShaderReadOnlyOptimal, aspectMask);
}

void OnlineTexture::createTextureImage(vk::CommandPool commandPool,
                                       vk::Queue transferQueue,
                                       vk::ImageUsageFlags moreFlags) {
    /* createImage(device.get(), w, h, vk::Format::eR8G8B8A8Srgb,
                 //  vk::ImageTiling::eOptimal is not working here?
                  vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eSampled,
                 vk::MemoryPropertyFlagBits::eHostVisible |
                     vk::MemoryPropertyFlagBits::eHostCoherent,
                 textureImage, textureImageMemory);

     transitionImageLayout(device->handle(), commandPool, transferQueue,
                           textureImage, vk::Format::eR8G8B8A8Srgb,
                           vk::ImageLayout::_UNDEFINED,
     vk::ImageLayout::_GENERAL);*/

    createImage(device.get(), w, h, format, vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransferDst |
                    vk::ImageUsageFlagBits::eSampled | moreFlags,
                vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage,
                textureImageMemory);

    buf = make_shared<Buffer>(device, w * h * 4,
                              vk::BufferUsageFlagBits::eTransferSrc,
                              vk::MemoryPropertyFlagBits::eHostVisible |
                                  vk::MemoryPropertyFlagBits::eHostCoherent);

    // Transition: undefined → transfer destination:
    layout = vk::ImageLayout::eUndefined;
    // transitionTo(vk::ImageLayout::eTransferDstOptimal); // TODO: Was this
    // line necessary?
}

void OnlineTexture::transitionTo(vk::ImageLayout l) {
    if (l != layout)
        transitionImageLayout(*device->device, commandPool, transferQueue,
                              *textureImage, format, layout, l, aspectMask);
    layout = l;
}

void OnlineTexture::transitionToRead() {
    // Transition: transfer destination → shader reading
    transitionTo(vk::ImageLayout::eShaderReadOnlyOptimal);
}

void OnlineTexture::update(const uint8_t *cpuData) {
    // Unfortunately the driver may not immediately copy the data into the
    // buffer memory, for example because of caching. It is also possible that
    // writes to the buffer are not visible in the mapped memory yet. There are
    // two ways to deal with that problem:
    //    - Use a memory heap that is host coherent, indicated with
    //      vk::MemoryPropertyFlagBits::eHostCoherent.
    //    - Call vkFlushMappedMemoryRanges after writing to the mapped memory,
    //      and call vkInvalidateMappedMemoryRanges before reading from the
    //      mapped memory (faster!).

    buf->copyFromCPU(cpuData);

    // Transition: undefined → transfer destination:
    transitionTo(vk::ImageLayout::eTransferDstOptimal);

    // Execute the buffer to image copy operation
    copyBufferToImage(device->handle(), commandPool, transferQueue,
                      buf->handle(), *textureImage, static_cast<uint32_t>(w),
                      static_cast<uint32_t>(h), layout, aspectMask);

    // Transition: transfer destination → shader reading
    transitionTo(vk::ImageLayout::eShaderReadOnlyOptimal);

    /*
        uint64_t size = w * h * 4;

        if (layout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            transitionImageLayout(device->handle(), commandPool, transferQueue,
                                  textureImage,format,
                                  vk::ImageLayout::eShaderReadOnlyOptimal,
                                  vk::ImageLayout::_GENERAL);
            layout = vk::ImageLayout::_GENERAL;
        }

        void *data;
        if (vkMapMemory(device->handle(), textureImageMemory, 0, size, 0, &data)
       != VK_SUCCESS) { throw runtime_error("couldn't map memory");
        }
        memcpy(data, cpuData, (size_t)size);
        vkUnmapMemory(device->handle(), textureImageMemory);

        if (layout != vk::ImageLayout::_GENERAL) {
            throw runtime_error("unexpected layout");
        }
        transitionImageLayout(device->handle(), commandPool, transferQueue,
                              textureImage, format,
                              vk::ImageLayout::_GENERAL,
                              vk::ImageLayout::eShaderReadOnlyOptimal);
        layout = vk::ImageLayout::eShaderReadOnlyOptimal;
        */
}

uint8_t *loadFile(const string &path, int &w, int &h) {
    // TODO:
    // vector<uint8_t> data = readFile2<uint8_t>(path);
    vector<uint8_t> data = fatouDB->getFile(path.c_str());
    return WebPDecodeRGBA(data.data(), data.size(), &w, &h);
}

void Sampler::createTextureSampler() {
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.sType = vk::StructureType::eSamplerCreateInfo;
    samplerInfo.magFilter = vk::Filter::eLinear;
    samplerInfo.minFilter = vk::Filter::eLinear;

    // - vk::SamplerAddressMode::eRepeat: Repeat the texture when going
    //   beyond the image dimensions.
    // - VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: Like repeat, but inverts
    //   the coordinates to mirror the image when going beyond the dimensions.
    // - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: Take the color of the edge
    //   closest to the coordinate beyond the image dimensions.
    // - VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: Like clamp to edge,
    //   but instead uses the edge opposite to the closest edge.
    // - VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: Return a solid color when
    //   sampling beyond the dimensions of the image.
    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;

    samplerInfo.anisotropyEnable = VK_TRUE;
    // lower value results in better performance, but lower quality results.
    // TODO: Maybe better not max this out...
    samplerInfo.maxAnisotropy = device->physical->properties.limits.maxSamplerAnisotropy;

    // It is possible to return black, white or transparent in either float
    // or int formats. You cannot specify an arbitrary color.
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;

    // coordinate system you want to use to address texels in an image. If
    // this field is VK_TRUE, then you can simply use coordinates within the
    // [0, texWidth) and [0, texHeight) range. If it is VK_FALSE, then the
    // texels are addressed using the [0, 1) range on all axes.
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    // If a comparison function is enabled, then texels will first be
    // compared to a value, and the result of that comparison is used in
    // filtering operations. This is mainly used for percentage-closer
    // filtering on shadow maps.
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;

    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    textureSampler = device->device.createSampler(samplerInfo);
}
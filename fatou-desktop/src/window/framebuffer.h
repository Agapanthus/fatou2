#pragma once

#include "logicalDevice.h"
#include "pipeline.h"
#include "shader.h"
#include "vulkanUtil.h"
#include "texture.h"

class FractalFramebuffer : private boost::noncopyable {
  public:
    FractalFramebuffer(shared_ptr<LogicalDevice> device,
                       vk::CommandPool commandPool,
                       shared_ptr<PipelineBase> pipeline,
                       vk::ImageUsageFlags moreFlags = {},
                       vk::Format stencilFormat = vk::Format::eUndefined)
        : device(device), extent(pipeline->extent), pipeline(pipeline),
          framebuffer(nullptr) {

        tex = make_shared<OnlineTexture>(
            device, commandPool, extent.width, extent.height,
            vk::ImageUsageFlagBits::eColorAttachment | moreFlags);
        tex->transitionToRead();
        vector<vk::ImageView> attachments = {tex->imageView()};

        if (stencilFormat != vk::Format::eUndefined) {
            depthStencilBuffer = make_shared<OnlineTexture>(
                device, commandPool, extent.width, extent.height,
                vk::ImageUsageFlagBits::eDepthStencilAttachment, stencilFormat,
                vk::ImageAspectFlagBits::eStencil |
                    vk::ImageAspectFlagBits::eDepth);
            depthStencilBuffer->transitionTo(
                vk::ImageLayout::eDepthStencilAttachmentOptimal);
            attachments.push_back(depthStencilBuffer->imageView());
        }

        // Create framebuffer

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;

        // You can only use a framebuffer with the render passes that it is
        // compatible with, which roughly means that they use the same number
        // and type of attachments.
        framebufferInfo.renderPass = pipeline->getRenderPass();

        framebufferInfo.attachmentCount = attachments.size();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;

        // Our swap chain images are single images, so the number of layers
        // is 1.
        framebufferInfo.layers = 1;

        framebuffer = device->device.createFramebuffer(framebufferInfo);
    }

    vk::ImageView imageView() { return tex->imageView(); }
    vk::Image image() { return tex->image(); }

    ~FractalFramebuffer() {
        std::cout << "Destroy frame buffer..." << std::endl;
        //  vkDestroyFramebuffer(device->handle(), framebuffer, nullptr);
    }

    Extent2D getExtent() { return extent; }
    shared_ptr<PipelineBase> getPipeline() { return pipeline; }
    shared_ptr<LogicalDevice> getDevice() { return device; }

    vk::Framebuffer getFramebuffer() { return *framebuffer; }

    void transition(vk::ImageLayout l) { tex->transitionTo(l); }

    void beforeRead() {
        // Too late to transition; it must be transitioned by now
        assert(tex->imageLayout() == vk::ImageLayout::eShaderReadOnlyOptimal);
        // tex->transitionToRead();
    }

    vk::ImageLayout imageLayout() { return tex->imageLayout(); }

  protected:
    shared_ptr<LogicalDevice> device;

    Extent2D extent;

    shared_ptr<OnlineTexture> tex;
    shared_ptr<OnlineTexture> depthStencilBuffer = nullptr;

    shared_ptr<PipelineBase> pipeline;

    vk::raii::Framebuffer framebuffer;
};
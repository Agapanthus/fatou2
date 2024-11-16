#pragma once

#include "commandBuffer.h"
#include "swapChain.h"
#include "vulkanUtil.h"

class RenderPassManager : private boost::noncopyable {
  public:
    RenderPassManager(CommandBufferRecorder &rec, const SwapChain &swapChain,
                      const size_t imageIndex)
        : commandBuffer(rec.commandBuffer) {

        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = vk::StructureType::eRenderPassBeginInfo;
        renderPassInfo.renderPass = swapChain.pipeline->getRenderPass();
        // We created a framebuffer for each swap chain image where it is
        // specified as a color attachment. Thus we need to bind the framebuffer
        // for the swapchain image we want to draw to. Using the imageIndex
        // parameter which was passed in, we can pick the right framebuffer for
        // the current swapchain image.
        renderPassInfo.framebuffer =
            *swapChain.framebuffers[imageIndex];

        // The next two parameters define the size of the render area. The
        // render area defines where shader loads and stores will take place.
        // The pixels outside this region will have undefined values. It should
        // match the size of the attachments for best performance.
        renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderPassInfo.renderArea.extent = swapChain.swapChainExtent;

        // The last two parameters define the clear values to use for
        // VK_ATTACHMENT_LOAD_OP_CLEAR, which we used as load operation for the
        // color attachment. I've defined the clear color to simply be black
        // with 100% opacity.
        vk::ClearValue clearColor =
            vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 1.0f}));
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        // The render pass can now begin. All of the functions that record
        // commands can be recognized by their vkCmd prefix. They all return
        // void, so there will be no error handling until we've finished
        // recording.
        // - VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be
        // embedded in the primary command buffer itself and no secondary
        // command buffers will be executed.
        // - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass
        // commands will be executed from secondary command buffers.
        commandBuffer.beginRenderPass(renderPassInfo,
                                      vk::SubpassContents::eInline);

        // Bind pipeline
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                   swapChain.pipeline->getGraphicsPipeline());
    }

    ~RenderPassManager() {
        // The render pass can now be ended:
        vkCmdEndRenderPass(commandBuffer);
    }

  private:
    vk::CommandBuffer commandBuffer;
};

class FractalRenderPassManager : private boost::noncopyable {
  public:
    FractalRenderPassManager(const CommandBufferRecorder &rec,
                             vk::Framebuffer framebuffer,
                             PipelineBase *pipeline)
        : commandBuffer(rec.commandBuffer) {

        vk::RenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = vk::StructureType::eRenderPassBeginInfo;
        renderPassInfo.renderPass = pipeline->getRenderPass();
        // We created a framebuffer for each swap chain image where it is
        // specified as a color attachment. Thus we need to bind the framebuffer
        // for the swapchain image we want to draw to. Using the imageIndex
        // parameter which was passed in, we can pick the right framebuffer for
        // the current swapchain image.
        renderPassInfo.framebuffer = framebuffer;

        // The next two parameters define the size of the render area. The
        // render area defines where shader loads and stores will take place.
        // The pixels outside this region will have undefined values. It should
        // match the size of the attachments for best performance.
        renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
        renderPassInfo.renderArea.extent = pipeline->extent;

        // The last two parameters define the clear values to use for
        // VK_ATTACHMENT_LOAD_OP_CLEAR, which we used as load operation for the
        // color attachment. I've defined the clear color to simply be black
        // with 100% opacity.
        std::array<vk::ClearValue, 2> clearColor;
        clearColor[0].color = vk::ClearColorValue(std::array<float,4>({0.0f, 0.0f, 0.0f, 1.0f}));
        renderPassInfo.clearValueCount = 1;
        if (pipeline->stencilMode != StencilMode::eNone) {
            renderPassInfo.clearValueCount = 2;
            clearColor[1].depthStencil = vk::ClearDepthStencilValue(0., 0);
        }
        renderPassInfo.pClearValues = clearColor.data();

        // The render pass can now begin. All of the functions that record
        // commands can be recognized by their vkCmd prefix. They all return
        // void, so there will be no error handling until we've finished
        // recording.
        // - VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be
        // embedded in the primary command buffer itself and no secondary
        // command buffers will be executed.
        // - VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass
        // commands will be executed from secondary command buffers.
        commandBuffer.beginRenderPass(renderPassInfo,
                                      vk::SubpassContents::eInline);

        // Bind pipeline
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                   pipeline->getGraphicsPipeline());
    }

    ~FractalRenderPassManager() {
        // The render pass can now be ended:
        vkCmdEndRenderPass(commandBuffer);
    }

  private:
    vk::CommandBuffer commandBuffer;
};

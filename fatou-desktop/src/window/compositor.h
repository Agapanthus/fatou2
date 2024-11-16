#pragma once

#include "ubo.h"
#include "pipeline.h"
#include "shader.h"
#include "nativeWindow.h"
#include "pingable.h"
#include "logicalDevice.h"
#include "swapChain.h"
#include "commandBuffer.h"
#include "renderPass.h"
#include "buffer.h"
#include "texture.h"
#include "compositor.h"
#include "sharedTexture.h"

class Compositor {
  public:
    Compositor(shared_ptr<LogicalDevice> device, vk::Queue transferQueue,
               shared_ptr<SwapChain> swapChain,
               shared_ptr<CommandPool> commandPool);

    shared_ptr<DescriptorPool> makeDP(vk::ImageView view) {
        return make_shared<DescriptorPool>(
            commandPool->MAX_FRAMES_IN_FLIGHT, device,
            swapChain->pipeline->descriptorSetLayout(), view,
            sampler->handle());
    }

    void begin(vk::CommandBuffer commandBuffer) {
        vb->bind(commandBuffer);
        ib->bind(commandBuffer);
    }

    void setSwapchain(shared_ptr<SwapChain> swapChain) {
        this->swapChain = swapChain;
    }

    void setTransform(DescriptorPool *pool, const glm::mat4 &mat4,
                      bool xflip = false) {

        UniformBufferObject ubo{};

        ubo.model = mat4;
        ubo.view = glm::mat4(1.0f);
        ubo.proj = glm::mat4(1.0f);

        // GLM was originally designed for OpenGL, where the Y coordinate of
        // the clip coordinates is inverted.
        if (xflip)
            ubo.proj[0][0] *= -1;
        else
            ubo.proj[1][1] *= -1;

        pool->update(commandPool->current(), swapChain->extent(), ubo);
    }

    void setTransform(DescriptorPool *pool, IRect r);

    void draw(vk::CommandBuffer commandBuffer, DescriptorPool *pool);

  private:
    shared_ptr<LogicalDevice> device;

    shared_ptr<VertexBuffer<Vertex2>> vb;
    shared_ptr<IndexBuffer> ib;

    shared_ptr<Sampler> sampler;

    shared_ptr<SwapChain> swapChain;
    shared_ptr<CommandPool> commandPool;
};
#pragma once

#include "shader.h"
#include "logicalDevice.h"
#include "ubo.h"
#include "framebuffer.h"

class PipelineWithDescriptorBase {
  public:
    virtual shared_ptr<FractalRenderPassManager>
    makeRPM(const CommandBufferRecorder &rec, vk::Framebuffer frameBuffer) = 0;
    virtual void updateVertex(const UniformBufferObject &ubo) = 0;

    virtual void updateFragment(const UniformBufferObject2 &ubo) = 0;
    virtual void bind(vk::CommandBuffer commandBuffer) = 0;
    virtual shared_ptr<PipelineBase> getPipeline() = 0;
};

template <class DSL, class DP>
class PipelineWithDescriptor : public PipelineWithDescriptorBase {
  public:
    PipelineWithDescriptor(shared_ptr<LogicalDevice> device,
                           const path &vertShader, const path &fragShader,
                           Extent2D extent, vk::CommandPool commandPool,
                           vk::ImageLayout initialLayout,
                           StencilMode stencilMode, vk::Format stencilFormat,
                           vector<vk::DynamicState> dynamicStates)
        : extent(extent), device(device) {

        pipeline = make_shared<Pipeline<DSL>>(
            device, make_shared<Shader>(device, vertShader, ShaderType::VERTEX),
            make_shared<Shader>(device, fragShader, ShaderType::FRAGMENT),
            extent, vk::Format::eR8G8B8A8Srgb, make_shared<DSL>(device),
            vk::ImageLayout::eShaderReadOnlyOptimal, initialLayout,
            stencilFormat, stencilMode, dynamicStates);

        dsl = make_shared<DSL>(device);
        descriptors = make_shared<DP>(device, *dsl->descriptorSetLayout);
    }

    shared_ptr<FractalRenderPassManager>
    makeRPM(const CommandBufferRecorder &rec,
            vk::Framebuffer frameBuffer) override {
        return make_shared<FractalRenderPassManager>(rec, frameBuffer,
                                                     &*pipeline);
    }

    void updateVertex(const UniformBufferObject &ubo) override {
        descriptors->updateVertex(ubo);
    }

    void updateFragment(const UniformBufferObject2 &ubo) override {
        descriptors->updateFragment(ubo);
    }

    void bind(vk::CommandBuffer commandBuffer) override {
        descriptors->bind(commandBuffer, pipeline->layout());
    }

    const Extent2D extent;

    shared_ptr<PipelineBase> getPipeline() override { return pipeline; }

  protected:
    shared_ptr<LogicalDevice> device;
    shared_ptr<Pipeline<DSL>> pipeline;
    shared_ptr<DP> descriptors;
    shared_ptr<DSL> dsl;
};

enum class MultiPipeMode { eSimple, eStencilWrite, eStencilRead, eEnd };

template <class DSL> class MultiPipe {
  public:
    MultiPipe(shared_ptr<LogicalDevice> device, const path &p, Extent2D extent,
              vk::CommandPool commandPool, vk::ImageUsageFlags moreFlags = {},
              vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
              bool withStencil = false)
        : extent(extent), device(device) {

        static vk::Format stencilFormat =
            getSupportedStencilFormat(&*device->physical);

        pipelines.resize(3);

        vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eScissor};

        pipelines[size_t(MultiPipeMode::eSimple)] =
            make_shared<PipelineWithDescriptor<
                DSL, DefaultDescriptorPool<UniformBufferObject,
                                           UniformBufferObject2>>>(
                device, shaderPath / "playground" / "simple.vert", p, extent,
                commandPool, initialLayout, StencilMode::eIgnore, stencilFormat,
                dynamicStates);

        pipelines[size_t(MultiPipeMode::eStencilRead)] =
            make_shared<PipelineWithDescriptor<
                DSL, DefaultDescriptorPool<UniformBufferObject,
                                           UniformBufferObject2>>>(
                device, shaderPath / "playground" / "simple.vert", p, extent,
                commandPool, initialLayout, StencilMode::eRead, stencilFormat,
                dynamicStates);

        pipelines[size_t(MultiPipeMode::eStencilWrite)] =
            make_shared<PipelineWithDescriptor<
                DescriptorSetLayoutVertexOnly,
                DefaultDescriptorPoolVertex<UniformBufferObject,
                                            UniformBufferObject2>>>(
                device, shaderPath / "playground" / "instanced.vert",
                shaderPath / "playground" / "white.frag", extent, commandPool,
                initialLayout, StencilMode::eWrite, stencilFormat,
                vector<vk::DynamicState>());

        frameBuffer = make_shared<FractalFramebuffer>(
            device, commandPool,
            pipelines[size_t(MultiPipeMode::eStencilRead)]->getPipeline(),
            moreFlags, stencilFormat);
    }

    shared_ptr<FractalRenderPassManager>
    makeRPM(const CommandBufferRecorder &rec, MultiPipeMode mode) {
        return pipelines[size_t(mode)]->makeRPM(rec,
                                                frameBuffer->getFramebuffer());
    }

    void updateVertex(const UniformBufferObject &ubo, MultiPipeMode mode) {
        pipelines[size_t(mode)]->updateVertex(ubo);
    }

    void updateFragment(const UniformBufferObject2 &ubo, MultiPipeMode mode) {
        pipelines[size_t(mode)]->updateFragment(ubo);
    }

    void bind(vk::CommandBuffer commandBuffer, MultiPipeMode mode) {
        pipelines[size_t(mode)]->bind(commandBuffer);
    }

    void beforeRead() { frameBuffer->beforeRead(); }
    void transition(vk::ImageLayout l) { frameBuffer->transition(l); }
    vk::ImageView imageView() { return frameBuffer->imageView(); }
    vk::Image image() { return frameBuffer->image(); }
    vk::ImageLayout imageLayout() { return frameBuffer->imageLayout(); }

    const Extent2D extent;

  protected:
    shared_ptr<LogicalDevice> device;
    shared_ptr<FractalFramebuffer> frameBuffer;
    vector<shared_ptr<PipelineWithDescriptorBase>> pipelines;
};
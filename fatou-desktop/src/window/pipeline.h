#pragma once

#include "shader.h"
#include "logicalDevice.h"
#include "ubo.h"

enum class StencilMode { eNone, eRead, eWrite, eIgnore };

class PipelineBase : private boost::noncopyable {
  public:
    PipelineBase(shared_ptr<LogicalDevice> device, shared_ptr<Shader> vert,
                 shared_ptr<Shader> frag, const Extent2D &extent,
                 vk::Format imageFormat, vk::Format depthFormat,
                 StencilMode stencilMode)
        : frag(frag), vert(vert), device(device), stencilMode(stencilMode),
          hasBlend(true), extent(extent), imageFormat(imageFormat),
          depthFormat(depthFormat) {}

    ~PipelineBase() {
        std::cout << "Destroy Pipeline..." << std::endl;
        // vkDestroyPipeline(device->handle(), graphicsPipeline, nullptr);
        // vkDestroyPipelineLayout(device->handle(), pipelineLayout, nullptr);
        // vkDestroyRenderPass(device->handle(), renderPass, nullptr);
    }

    shared_ptr<LogicalDevice> getDevice() { return device; };
    vk::RenderPass getRenderPass() { return *renderPass; };
    vk::Pipeline getGraphicsPipeline() { return *graphicsPipeline; };

    virtual vk::DescriptorSetLayout descriptorSetLayout() const = 0;
    vk::PipelineLayout layout() const { return *pipelineLayout; }

  protected:
    shared_ptr<Shader> frag;
    shared_ptr<Shader> vert;
    vk::raii::PipelineLayout pipelineLayout = nullptr;
    shared_ptr<LogicalDevice> device;

    vk::raii::RenderPass renderPass = nullptr;
    vk::raii::Pipeline graphicsPipeline = nullptr;

  public:
    const Extent2D extent;
    const vk::Format imageFormat;
    const vk::Format depthFormat;
    const StencilMode stencilMode;
    const bool hasBlend;

  protected:
    void createPipeline(vk::ImageLayout finalLayout,
                        vk::ImageLayout initialLayout,
                        vector<vk::DynamicState> dynamicStates);
    void createRenderPass(vk::ImageLayout finalLayout,
                          vk::ImageLayout initialLayout);
    void createPipelineLayout(const vk::DescriptorSetLayout *altDsl);

    void setupInputAssembly(vk::PipelineInputAssemblyStateCreateInfo &);
    void setupViewport(const Extent2D &swapChainExtent, vk::Rect2D &scissor,
                       vk::Viewport &viewport,
                       vk::PipelineViewportStateCreateInfo &viewportState);
    void setupRasterizer(vk::PipelineRasterizationStateCreateInfo &);
    void setupMultisampling(vk::PipelineMultisampleStateCreateInfo &);
    void setupStencil(vk::PipelineDepthStencilStateCreateInfo &);
    void
    setupBlending(vk::PipelineColorBlendAttachmentState &colorBlendAttachment,
                  vk::PipelineColorBlendStateCreateInfo &colorBlending);
};

template <class DSL> class Pipeline : public PipelineBase {
  public:
    Pipeline(shared_ptr<LogicalDevice> device, shared_ptr<Shader> vert,
             shared_ptr<Shader> frag, const Extent2D &extent,
             vk::Format imageFormat, shared_ptr<DSL> dsl,
             vk::ImageLayout finalLayout,
             vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined,
             vk::Format depthFormat = vk::Format::eUndefined,
             StencilMode stencilMode = StencilMode::eNone,
             vector<vk::DynamicState> dynamicStates = {})
        : PipelineBase(device, vert, frag, extent, imageFormat, depthFormat,
                       stencilMode),
          dsl(dsl) {
        assert(bool(finalLayout));
        createPipelineLayout(&*dsl->descriptorSetLayout);
        createPipeline(finalLayout, initialLayout, dynamicStates);
    }

    virtual vk::DescriptorSetLayout descriptorSetLayout() const override {
        return *dsl->descriptorSetLayout;
    }

  protected:
    shared_ptr<DSL> dsl;
};

/*
template <class DSL>
class FractalPipe : public Pipeline<DSL> {

  public:
    FractalPipe(shared_ptr<LogicalDevice> device, shared_ptr<Shader> vert,
                shared_ptr<Shader> frag, const Extent2D &extent,
                const vk::Format &imageFormat,
                shared_ptr<MandelDescriptorSetLayout> mandelDsl)
        : mandelDsl(mandelDsl),
          Pipeline(device, vert, frag, extent, imageFormat,
                   &(mandelDsl->descriptorSetLayout),
                   vk::ImageLayout::eShaderReadOnlyOptimal) {}

    vk::DescriptorSetLayout descriptorSetLayout() const override {
        return *mandelDsl->descriptorSetLayout;
    }

  private:
    shared_ptr<MandelDescriptorSetLayout> mandelDsl;
};*/
#pragma once

#include "texture.h"
#include "framebuffer.h"
#include "renderPass.h"
#include "pipelineWithTarget.h"

extern const std::vector<Vertex> vertices;
extern const std::vector<uint16_t> indices;
extern const std::vector<Vertex2> vertices2;

/*
template <class DSL> class OffRenderer {
  public:
    OffRenderer(shared_ptr<LogicalDevice> device, const path &path,
                       Extent2D extent, vk::CommandPool commandPool)
        : device(device), extent(extent) {

        pipeline = make_shared<PipelineWithTarget<DSL>>(device, path, extent,
                                                        commandPool);

        vb = make_shared<VertexBuffer<Vertex2>>(device, vertices2, commandPool);
        ib = make_shared<IndexBuffer>(device, indices, commandPool);
    }

    void renderStep(const CommandBufferRecorder &rec,
                    vk::CommandBuffer commandBuffer) {
        const auto rpm = this->makeRPM(rec);

        updatePerspective();

        ///////////////////////////

        vb->bind(commandBuffer);
        ib->bind(commandBuffer);

        pipeline->bind(commandBuffer);

        vkCmdDrawIndexed(
            commandBuffer,
            static_cast<uint32_t>(indices.size()), // number of indices
            1,                                     // number of instances
            0, // first index from the buffer
            0, // offset to add to the indices
            0  // first instance
        );
    }

    Extent2D getExtent() { return pipeline->getExtent(); }
    vk::ImageView imageView() { return pipeline->imageView(); }

    void updateFragment(const UniformBufferObject2 &ubo) {
        pipeline->updateFragment(ubo);
    }

  private:
    shared_ptr<FractalRenderPassManager>
    makeRPM(const CommandBufferRecorder &rec) {
        return  pipeline->makeRPM(rec);
    }

    void updatePerspective() {

        // TODO: interlaced renderer should use a stack of scaled interlaced
        // projections, each with its own Framebuffer and Pipeline

        UniformBufferObject ubo{};
        const Extent2D e = pipeline->getExtent();
        ubo.model = glm::scale(
            glm::mat4(1.0f),
            glm::vec3(std::max(1.0f, e.height / float(e.width)),
                      std::max(1.0f, e.width / float(e.height)), 1.0f));

        ubo.view = glm::mat4(1.0f);
        // ubo2.view[1][1] *=
        //     swapChain->extent().width / float(swapChain->extent().height);
        // good: 1080, 1072
        //
        ubo.proj = glm::mat4(1.0f);
        ubo.proj[0][0] *= -1;

        pipeline->updateVertex(ubo);
    }

  private:
    shared_ptr<LogicalDevice> device;

    shared_ptr<VertexBuffer<Vertex2>> vb;
    shared_ptr<IndexBuffer> ib;

    shared_ptr<PipelineWithTarget<DSL>> pipeline;

    Extent2D extent;
};*/
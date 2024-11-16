#pragma once

#include "texture.h"
#include "pipelineWithTarget.h"
#include "renderPass.h"
#include "offRenderer.h"
#include "compositor.h"
#include "measurePerformance.h"
#include "pingable.h"
#include "timing.h"
#include "commandBuffer.h"
#include "../gui/cef/js.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

template <class DSL> class InterlacedRenderer {
  public:
    InterlacedRenderer(shared_ptr<LogicalDevice> device, const path &path,
                       Extent2D extent, shared_ptr<CommandPool> commandPool,
                       size_t phases)
        : device(device), extent(extent), commandPool(commandPool),
          timer(device, phases) {

        vb = make_shared<VertexBuffer<Vertex2>>(device, vertices2,
                                                commandPool->renderer());
        ib = make_shared<IndexBuffer>(device, indices, commandPool->renderer());

        createFramebuffers(path);
        initStencil();
        invalidate();
    }

    void createFramebuffers(const path &path) {
        int width = extent.width;
        int height = extent.height;
        const int minArea = 1000;
        maxLayer = 0;

        // Fill in pattern:
        //
        // 1 2 1 2 | 1 2 1 2
        // 1 3 1 4 | 1 3 1 4
        // 1 2 1 2 | 1 2 1 2
        // 1 3 1 5 | 1 3 1 5
        // --------+--------
        // 1 2 1 2 | 1 2 1 2
        // 1 3 1 4 | 1 3 1 4
        // 1 2 1 2 | 1 2 1 2
        // 1 3 1 5 | 1 3 1 5
        //
        // But interpolation positions are centered at the largest containing
        // cell; therefore, translation adjustments are necessary
        // Furthermore, when the size is not even, an additional row/column
        // is added; therefore, also the scaling is inconsistent

        // part cut at the end due to odd side lengths
        int cutoffX = 0;
        int cutoffY = 0;

        int pixSizeX = 1;
        int pixSizeY = 1;

        int x = 0;
        int y = 0;

        while (width * height > minArea) {
            // TODO: Share the pipeline between buffers! this might be faster!
            /*
            Sascha Willems:
            If your framebuffers comply to the compatibility rules then you can
            reuse a pipeline and renderpass with different framebuffers, and
            that should work fine.

            Another option is to use a layered framebuffer with each layer
            representing a different shadow map, and either use geometry shaders
            (when available) or multiview to render to the different layers.
            This will only require a single renderpass and may be faster.

            I do have samples for both in my repository.
            */
            /*
            8.2. Render Pass Compatibility

            Framebuffers and graphics pipelines are created based on a specific
            render pass object. They must only be used with that render pass
            object, or one compatible with it.

            Two attachment references are compatible if they have matching
            format and sample count, or are both VK_ATTACHMENT_UNUSED or the
            pointer that would contain the reference is NULL.

            Two arrays of attachment references are compatible if all
            corresponding pairs of attachments are compatible. If the arrays are
            of different lengths, attachment references not present in the
            smaller array are treated as VK_ATTACHMENT_UNUSED.

            Two render passes are compatible if their corresponding color,
            input, resolve, and depth/stencil attachment references are
            compatible and if they are otherwise identical except for:

                Initial and final image layout in attachment descriptions

                Load and store operations in attachment descriptions

                Image layout in attachment references

            As an additional special case, if two render passes have a single
            subpass, the resolve attachment reference and depth/stencil resolve
            mode compatibility requirements are ignored.

            A framebuffer is compatible with a render pass if it was created
            using the same render pass or a compatible render pass.

            */

            pipeline.push_back(make_shared<MultiPipe<DSL>>(
                device, path, Extent2D(width, height), commandPool->transfer(),
                vk::ImageUsageFlagBits::eTransferSrc |
                    vk::ImageUsageFlagBits::eTransferDst,
                vk::ImageLayout::eShaderReadOnlyOptimal, true));

            pushXYWH(x, y, cutoffX, cutoffY);
            if (width % 2 == 1) {
                cutoffX += pixSizeX;
            }
            x += pixSizeX;
            width /= 2;
            pixSizeX *= 2;

            pipeline.push_back(make_shared<MultiPipe<DSL>>(
                device, path, Extent2D(width, height), commandPool->transfer(),
                vk::ImageUsageFlagBits::eTransferSrc |
                    vk::ImageUsageFlagBits::eTransferDst,
                vk::ImageLayout::eShaderReadOnlyOptimal, true));

            pushXYWH(x, y, cutoffX, cutoffY);
            if (height % 2 == 1) {
                cutoffY += pixSizeY;
            }
            y += pixSizeY;
            height /= 2;
            pixSizeY *= 2;

            maxLayer += 2;
        }
    }

    // stores transforms for a framebuffer (for usage in updatePerspective)
    void pushXYWH(int x, int y, int cutoffX, int cutoffY) {
        const double ow = std::max(1.0f, extent.height / float(extent.width));
        const double oh = std::max(1.0f, extent.width / float(extent.height));
        const int maxD = std::max(extent.width, extent.height);

        const double dx = maxD / double(maxD - cutoffX);
        const double dy = maxD / double(maxD - cutoffY);

        const double scale = 1. / maxD;

        xs.push_back((x - cutoffX) * scale);
        ys.push_back((y - cutoffY) * scale);
        ws.push_back(ow * dx);
        hs.push_back(oh * dy);
    }

    // writs horizontal or vertical 1px-bars to the stencil buffers
    void initStencil() {

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
        allocInfo.level = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandPool = commandPool->renderer();
        allocInfo.commandBufferCount = 1;

        vector<vk::raii::CommandBuffer> commandBuffers =
            device->device.allocateCommandBuffers(allocInfo);

        UniformBufferObject ubo{};
        ubo.view = glm::mat4(1.0f);
        ubo.proj = glm::mat4(1.0f);
        ubo.proj[0][0] *= -1;

        {
            CommandBufferRecorder rec(*commandBuffers[0]);

            bool isHori = false;
            for (size_t i = 0; i < pipeline.size(); i++) {
                isHori = !isHori;

                const auto rpm = makeRPM(rec, i, MultiPipeMode::eStencilWrite);
                {
                    vb->bind(*commandBuffers[0]);
                    ib->bind(*commandBuffers[0]);

                    pipeline[i]->bind(*commandBuffers[0],
                                      MultiPipeMode::eStencilWrite);

                    size_t mj = 0;
                    if (isHori) {
                        mj = pipeline[i]->extent.width;
                    } else {
                        mj = pipeline[i]->extent.height;
                    }

                    if (i == pipeline.size() - 1) {
                        // last (smallest) buffer; this should be a full quad

                        ubo.model = glm::scale(
                            glm::mat4(1.0f),
                            glm::vec3(std::max(1.0f, extent.height /
                                                         float(extent.width)),
                                      std::max(1.0f, extent.width /
                                                         float(extent.height)),
                                      1.0f));
                        ubo.view[0][0] = 0;
                        ubo.view[0][1] = 0;

                        pipeline[i]->updateVertex(ubo,
                                                  MultiPipeMode::eStencilWrite);

                        vkCmdDrawIndexed(
                            *commandBuffers[0],
                            static_cast<uint32_t>(
                                indices.size()), // number of indices
                            1,                   // number of instances
                            0,                   // first index from the buffer
                            0,                   // offset to add to the indices
                            0                    // first instance
                        );

                    } else {
                        // horizontal or vertical 1px bars
                        // can't use fragment shader because we need the result
                        // in the stencil buffer

                        float d = 1.0f / mj;

                        if (isHori) {
                            ubo.model = glm::translate(
                                glm::scale(
                                    glm::mat4(1.0f),
                                    glm::vec3(
                                        d,
                                        std::max(1.0f,
                                                 extent.width /
                                                     float(extent.height)),
                                        1.0f)),
                                glm::vec3(float(mj - 2), 0.f, 0.f));
                            ubo.view[0][0] = d * 4.;
                            ubo.view[0][1] = 0;
                        } else {
                            ubo.model = glm::translate(
                                glm::scale(
                                    glm::mat4(1.0f),
                                    glm::vec3(
                                        std::max(1.0f, extent.height /
                                                           float(extent.width)),
                                        d, 1.0f)),
                                glm::vec3(0.f, -float(mj - 2), 0.f));
                            ubo.view[0][0] = 0;
                            ubo.view[0][1] = d * 4.;
                        }

                        pipeline[i]->updateVertex(ubo,
                                                  MultiPipeMode::eStencilWrite);

                        ///////////////////////////

                        vkCmdDrawIndexed(
                            *commandBuffers[0],
                            static_cast<uint32_t>(
                                indices.size()), // number of indices
                            (mj + 1) / 2,        // #instances, +1 if odd size
                            0,                   // first index from the buffer
                            0,                   // offset to add to the indices
                            0                    // first instance
                        );
                    }
                }
            }
        }

        vk::SubmitInfo submitInfo{};
        submitInfo.sType = vk::StructureType::eSubmitInfo;
        submitInfo.commandBufferCount = commandBuffers.size();
        submitInfo.pCommandBuffers = &*commandBuffers[0];
        device->graphicsQueue.submit(submitInfo, {});

        device->graphicsQueue.waitIdle();
    }

    void invalidate() {
        // TODO: restore from buffer translation, i.e. don't discard everything
        // if not necessary!

        // back in max layer
        finishedLayer = maxLayer;
        currentProg = 0;
    }

    void hardInvalidate() {
        invalidate();
        estim.reset();
    }

    void makeDP(Compositor &compositor) {
        for (size_t i = 0; i < maxLayer; i++) {
            presentationDescriptorPools.push_back(
                compositor.makeDP(pipeline[i]->imageView()));
        }
    }

    void present(vk::CommandBuffer commandBuffer, Compositor &compositor,
                 IRect r) {
        size_t i = std::min(maxLayer - 1, finishedLayer);
        if (i >= 0 && finishedLayer != maxLayer) {
            if (currentProg > 0) {
                i -= 1;
            }
        }
        const auto x = pipeline[i]->imageLayout();
        pipeline[i]->beforeRead();
        compositor.setTransform(presentationDescriptorPools[i].get(), r);
        compositor.draw(commandBuffer, presentationDescriptorPools[i].get());
    }

    void renderStep(const CommandBufferRecorder &rec,
                    vk::CommandBuffer commandBuffer,
                    const UniformBufferObject2 &ubo2, size_t bufferIndex) {

        // fetch the last value before re-submitting it
        timer.fetch(bufferIndex);

        auto p = timer.getTime(bufferIndex);
        double renderTime = 0.;
        if (p.has_value()) {
            auto [r, u] = p.value();
            renderTime = r;
            if (r > 0.05) {
                // Slow frame
                std::cout << formatBig(int(1.0 / (r / u))) << "s/s ("
                          << formatBig(u) << " samples took " << r * 1000
                          << "ms)" << std::endl;
            }
            estim.push(r, u, 1.0);
        }

        const int64_t targetEffort = estim.predictSamplesLinear(1. / 50);
        int64_t samples = targetEffort;
        samples = std::min(int64_t(1000 * 1000 * 100),
                           std::max(int64_t(1000), samples));
        if (p.has_value()) {
            // std::cout << formatBig(samples) << std::endl;
        }

        int l = maxLayer - 1;
        // TODO: Da muss eine barrier um das copyBufferLayer damit das
        // funktioniert... (ich will hier mehrere Iterationen in einem Frame
        // submitten. Vielleicht sollte der Renderer einen eigenen Puffer
        // verwenden)
        // while (samples >= pipeline[l]->extent.height * 2)
        {

            if (finishedLayer == 0)
                return;
            l = finishedLayer - 1;

            // TODO: sometimes you can still artifacts from the entry layer when
            // switching between presets!

            // TODO: something is wrong. Replacing this to eSimple should make
            // it exactly twice at slow, but it doesn't change anything
            MultiPipeMode mode = MultiPipeMode::eStencilRead;
            assert(l <= maxLayer - 1);
            if (l == maxLayer - 1) {
                mode = MultiPipeMode::eSimple;
                if (currentProg == 0) {
                    while (l >= 1 && samples / pipeline[l - 1]->extent.height >=
                                         pipeline[l - 1]->extent.width) {
                        l--;
                    }
                    const auto e = pipeline[l]->extent;
                    assert(l == maxLayer - 1 || samples / e.height >= e.width);
                    finishedLayer = l + 1;
                    // std::cout << "skipping to " << l << std::endl;
                }
            }

            if (mode != MultiPipeMode::eSimple && currentProg == 0) {
                // MeasurePerformance("interlaced blit");
                copyBufferLayer(l, l + 1);
                // std::cout << "copied " << l << std::endl;
            }

            this->updateFragment(ubo2, l, mode);
            const auto renderedSamples = this->renderStep(
                rec, commandBuffer, l, bufferIndex, samples, mode);
            samples -= renderedSamples;
            // std::cout << "rendered " << l << " with " << renderedSamples
            //           << std::endl;

            assert(mode != MultiPipeMode::eSimple || l == maxLayer - 1 ||
                   currentProg == pipeline[l]->extent.width);

            if (currentProg == pipeline[l]->extent.width) {
                currentProg = 0;
                finishedLayer = l;
            }
        }

        commitJS("setRenderParams",
                 "{currentProgress:" +
                     jsStrD(double(currentProg) / pipeline[l]->extent.width) +
                     ",finishedLayer:" + jsStr(finishedLayer) +
                     ",width:" + jsStr(extent.width) +
                     ",height:" + jsStr(extent.height) + ",targetEffort:" +
                     jsStr((finishedLayer != 0) ? targetEffort : 0) +
                     ",renderTime:" + jsStrD(renderTime) + "}");
    }

    void copyBufferLayer(size_t i, size_t j) {
        // j is src, i is dst

        pipeline[j]->transition(vk::ImageLayout::eTransferSrcOptimal);
        pipeline[i]->transition(vk::ImageLayout::eTransferDstOptimal);

        // TODO: do all of this in one pipeline? all this "waitIdle"
        // will take too much time! But, does it? MeasurePerformance
        // says <2us
        {
            SingleTimeCommandManager manager(commandPool->renderer(),
                                             *device->device,
                                             *device->transferQueue);

            std::array<vk::ImageBlit, 1> regions;
            regions[0].srcOffsets[0].x = 0;
            regions[0].srcOffsets[0].y = 0;
            regions[0].srcOffsets[0].z = 0;
            regions[0].srcOffsets[1].x = pipeline[j]->extent.width;
            regions[0].srcOffsets[1].y = pipeline[j]->extent.height;
            regions[0].srcOffsets[1].z = 1;
            regions[0].srcSubresource.aspectMask =
                vk::ImageAspectFlagBits::eColor;
            regions[0].srcSubresource.layerCount = 1;
            regions[0].srcSubresource.mipLevel = 0;
            regions[0].dstOffsets[0].x = 0;
            regions[0].dstOffsets[0].y = 0;
            regions[0].dstOffsets[0].z = 0;
            regions[0].dstOffsets[1].x = pipeline[i]->extent.width;
            regions[0].dstOffsets[1].y = pipeline[i]->extent.height;
            regions[0].dstOffsets[1].z = 1;
            regions[0].dstSubresource.aspectMask =
                vk::ImageAspectFlagBits::eColor;
            regions[0].dstSubresource.layerCount = 1;
            regions[0].dstSubresource.mipLevel = 0;

            manager.commandBuffers[0].blitImage(
                pipeline[j]->image(), vk::ImageLayout::eTransferSrcOptimal,
                pipeline[i]->image(), vk::ImageLayout::eTransferDstOptimal,
                regions, vk::Filter::eNearest);
        }

        // Doing this in the same buffer interfers with blitImage
        pipeline[j]->transition(vk::ImageLayout::eShaderReadOnlyOptimal);
        pipeline[i]->transition(vk::ImageLayout::eShaderReadOnlyOptimal);
    }

    uint64_t renderStep(const CommandBufferRecorder &rec,
                        vk::CommandBuffer commandBuffer, size_t l,
                        size_t bufferIndex, uint64_t effort,
                        MultiPipeMode mode) {
        checkLayer(l);

        const auto e = pipeline[l]->extent;
        // uint64_t effort = e.width * e.height;

        // TODO: ignore stencil buffered lines in a smarter way!
        assert(currentProg < e.width);
        // min is 2, because the thing is interlaced and 1 and 2 are the same effort
        uint64_t lines =
            std::min(e.width - currentProg,
                     std::max(uint64_t(2), uint64_t((effort * 2) / e.height)));
        assert(lines > 0);
        if (mode == MultiPipeMode::eSimple)
            assert(l == maxLayer - 1 || (currentProg == 0 && lines == e.width));

        effort = lines * e.height;

        if (l != maxLayer - 1) {
            effort /= 2;
        }
        timer.start(commandBuffer, bufferIndex, effort);
        {
            const auto rpm = this->makeRPM(rec, l, mode);

            updatePerspective(l, mode);

            ///////////////////////////

            vb->bind(commandBuffer);
            ib->bind(commandBuffer);

            pipeline[l]->bind(commandBuffer, mode);

            vk::Rect2D scissor;
            scissor.offset.x = currentProg;
            scissor.offset.y = 0;
            scissor.extent.width = lines;
            scissor.extent.height = pipeline[l]->extent.height;
            currentProg += lines;
            assert(currentProg <= pipeline[l]->extent.width);

            commandBuffer.setScissor(0, 1, &scissor);

            vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(indices.size()), // number of indices
                1,                                     // number of instances
                0, // first index from the buffer
                0, // offset to add to the indices
                0  // first instance
            );
        }
        timer.stop(commandBuffer, bufferIndex);

        return effort;
    }

    Extent2D getExtent(size_t i = 0) {
        checkLayer(i);
        return pipeline[i]->getExtent();
    }

    vk::ImageView imageView(size_t i = 0) {
        checkLayer(i);
        return pipeline[i]->imageView();
    }

    void updateFragment(const UniformBufferObject2 &ubo, size_t i,
                        MultiPipeMode mode) {
        checkLayer(i);
        pipeline[i]->updateFragment(ubo, mode);
    }

  private:
    inline void checkLayer(size_t i) { assert(i < maxLayer); }

    shared_ptr<FractalRenderPassManager>
    makeRPM(const CommandBufferRecorder &rec, size_t i, MultiPipeMode mode) {
        checkLayer(i);
        return pipeline[i]->makeRPM(rec, mode);
    }

    void updatePerspective(size_t i, MultiPipeMode mode) {
        checkLayer(i);

        UniformBufferObject ubo{};
        ubo.model = glm::translate(
            glm::scale(glm::mat4(1.0f), glm::vec3(ws[i], hs[i], 1.0f)),
            glm::vec3(xs[i], -ys[i], 0.));
        ubo.view = glm::mat4(1.0f);
        ubo.proj = glm::mat4(1.0f);
        ubo.proj[0][0] *= -1;

        pipeline[i]->updateVertex(ubo, mode);
    }

  private:
    shared_ptr<LogicalDevice> device;

    TimeQueryPool timer;

    shared_ptr<VertexBuffer<Vertex2>> vb;
    shared_ptr<IndexBuffer> ib;
    shared_ptr<CommandPool> commandPool;

    vector<shared_ptr<MultiPipe<DSL>>> pipeline;

    vector<shared_ptr<DescriptorPool>> presentationDescriptorPools;

    vector<double> xs;
    vector<double> ys;
    vector<double> ws;
    vector<double> hs;

    Extent2D extent;

    size_t finishedLayer = 0;
    size_t maxLayer = 0;
    size_t currentProg = 0;

    EffortEstimator estim;
};
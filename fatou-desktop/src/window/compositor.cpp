#include "compositor.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "fractal.h"

Compositor::Compositor(shared_ptr<LogicalDevice> device,
                       vk::Queue transferQueue, shared_ptr<SwapChain> swapChain,
                       shared_ptr<CommandPool> commandPool)
    : device(device), swapChain(swapChain), commandPool(commandPool) {

    vb = make_shared<VertexBuffer<Vertex2>>(device, vertices2,
                                           commandPool->transfer());
    ib = make_shared<IndexBuffer>(device, indices, commandPool->transfer());

    sampler = make_shared<Sampler>(device);
}

void Compositor::draw(vk::CommandBuffer commandBuffer, DescriptorPool *pool) {
    pool->bind(commandBuffer, commandPool->current(),
               swapChain->pipeline->layout());

    vkCmdDrawIndexed(commandBuffer,
                     static_cast<uint32_t>(indices.size()), // number of indices
                     1, // number of instances
                     0, // first index from the buffer
                     0, // offset to add to the indices
                     0  // first instance
    );
}

void Compositor::setTransform(DescriptorPool *pool, IRect r) {
    int w = r.right - r.left;
    int h = r.bottom - r.top;
    float dx = w / float(swapChain->extent().width);
    float dy = h / float(swapChain->extent().height);
    UniformBufferObject ubo{};
    ubo.model = glm::translate(
        glm::scale(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(-(-1.0f + dx +
                            2.0f * r.left / float(swapChain->extent().width)),
                          (-1.0f + dy +
                           2.0f * r.top / float(swapChain->extent().height)),
                          .0f)),
            glm::vec3(dx, dy, 1.0f)),
        glm::vec3(.0f, .0f, .0f));
    // glm::vec3(-1.0f / dx, -1.0f / dy, .0f));
    // glm::vec3(r.left / float(swapChain->extent().width), r.top /
    // float(swapChain->extent().height), .0f));
    ubo.view = glm::mat4(1.0f);
    ubo.proj = glm::mat4(1.0f);
    ubo.proj[0][0] *= -1;
    pool->update(commandPool->current(), swapChain->extent(), ubo);
}
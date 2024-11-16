#pragma once

#include "logicalDevice.h"
#include "semaphore.h"
#include "fence.h"

class CommandPool : private boost::noncopyable {
  public:
    // problem: We are required to wait on the previous frame to finish before
    // we can start rendering the next which results in unnecessary idling of
    // the host. The way to fix this is to allow multiple frames to be in-flight
    // at once, that is to say, allow the rendering of one frame to not
    // interfere with the recording of the next.
    const int MAX_FRAMES_IN_FLIGHT = 2;

  public:
    CommandPool(shared_ptr<LogicalDevice> device) : device(device) {
        createCommandPool();
        createCommandBuffers();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            inFlightFences.push_back(make_shared<Fence>(device, true));
            imageAvailableSemaphores.push_back(make_shared<Semaphore>(device));
            renderFinishedSemaphores.push_back(make_shared<Semaphore>(device));
        }
    }

    ~CommandPool() {
        // vkDestroyCommandPool(device->handle(), commandPool, nullptr);
        // vkDestroyCommandPool(device->handle(), commandPoolTransfer, nullptr);
    }

    vk::CommandBuffer buffer(size_t i) { return *commandBuffers[i]; }

    void submitCommandBuffer();

    void swapBuffers() {
        // advance to next frame
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void resetCommandBuffer() { commandBuffers[currentFrame].reset(); }

    vk::CommandBuffer currentBuffer() { return *commandBuffers[currentFrame]; }

    void presentFrame(const vk::raii::SwapchainKHR &swapChainHandle,
                      const uint32_t imageIndex);

    optional<uint32_t>
    acquireNextImage(const vk::raii::SwapchainKHR &swapChainHandle, bool abort);

    vk::CommandPool transfer() { return *commandPoolTransfer; }
    vk::CommandPool renderer() { return *commandPoolRenderer; }

    uint32_t current() const { return currentFrame; }

    const vector<vk::CommandBuffer> &getCommandBuffers() {
        return simpleCommandBuffers;
    }

  private:
    void createCommandPool();
    void createCommandBuffers();

  private:
    vk::raii::CommandPool commandPool = nullptr;
    vector<vk::raii::CommandBuffer> commandBuffers;
    vector<vk::CommandBuffer> simpleCommandBuffers;
    shared_ptr<LogicalDevice> device;

    vk::raii::CommandPool commandPoolTransfer = nullptr;
    vk::raii::CommandPool commandPoolRenderer = nullptr;

    uint32_t currentFrame = 0;

    vector<shared_ptr<Semaphore>> imageAvailableSemaphores;
    vector<shared_ptr<Semaphore>> renderFinishedSemaphores;
    vector<shared_ptr<Fence>> inFlightFences;
};

class CommandBufferRecorder : private boost::noncopyable {
  public:
    CommandBufferRecorder(const vk::CommandBuffer &commandBuffer)
        : commandBuffer(commandBuffer) {
        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.sType = vk::StructureType::eCommandBufferBeginInfo;

        // The flags parameter specifies how we're going to use the command
        // buffer. The following values are available:
        // - vk::CommandBufferUsageFlagBits::eOneTimeSubmit: The command buffer
        // will be rerecorded right after executing it once.
        // - VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a
        // secondary command buffer that will be entirely within a single render
        // pass.
        // - VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer
        // can be resubmitted while it is also already pending execution.
        beginInfo.flags = {}; // Optional

        // The pInheritanceInfo parameter is only relevant for secondary command
        // buffers. It specifies which state to inherit from the calling primary
        // command buffers.
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // If the command buffer was already recorded once, then a call to
        // vkBeginCommandBuffer will implicitly reset it. It's not possible to
        // append commands to a buffer at a later time.
        commandBuffer.begin(beginInfo);
    }

    ~CommandBufferRecorder() {
        // And we've finished recording the command buffer:
        commandBuffer.end();
    }

  private:
    const vk::CommandBuffer &commandBuffer;

    friend class RenderPassManager;
    friend class FractalRenderPassManager;
};
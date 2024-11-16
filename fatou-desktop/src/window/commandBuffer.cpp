#include "commandBuffer.h"

void CommandPool::presentFrame(const vk::raii::SwapchainKHR &swapChainHandle,
                               const uint32_t imageIndex) {
    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType = vk::StructureType::ePresentInfoKHR;

    vk::Semaphore signalSemaphores[] = {
        *renderFinishedSemaphores[currentFrame]->handle};

    // The first two parameters specify which semaphores to wait on before
    // presentation can happen, just like vk::SubmitInfo. Since we want to
    // wait on the command buffer to finish execution, thus our triangle
    // being drawn, we take the semaphores which will be signalled and wait
    // on them, thus we use signalSemaphores.
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    // The next two parameters specify the swap chains to present images to
    // and the index of the image for each swap chain. This will almost
    // always be a single one.
    vk::SwapchainKHR swapChains[] = {*swapChainHandle};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    // There is one last optional parameter called pResults. It allows you
    // to specify an array of vk::Result values to check for every individual
    // swap chain if presentation was successful. It's not necessary if
    // you're only using a single swap chain, because you can simply use the
    // return value of the present function.
    presentInfo.pResults = nullptr; // Optional

    // The vkQueuePresentKHR function submits the request to present an
    // image to the swap chain. We'll add error handling for both
    // vkAcquireNextImageKHR and vkQueuePresentKHR in the next chapter,
    // because their failure does not necessarily mean that the program
    // should terminate, unlike the functions we've seen so far
    vk::Result result = device->presentQueue.presentKHR(presentInfo);
    if (result != vk::Result::eSuccess) {
        // TODO
    }
}

optional<uint32_t>
CommandPool::acquireNextImage(const vk::raii::SwapchainKHR &swapChainHandle,
                              bool abort) {
    // At the start of the frame, we want to wait until the previous frame
    // has finished
    inFlightFences[currentFrame]->wait();

    // The third parameter specifies a timeout in nanoseconds for an image
    // to become available. The next two parameters specify synchronization
    // objects that are to be signaled when the presentation engine is
    // finished using the image. The last parameter specifies a variable to
    // output the index of the swap chain image that has become available.
    auto [result, imageIndex] = swapChainHandle.acquireNextImage(
        UINT64_MAX, *imageAvailableSemaphores[currentFrame]->handle,
        VK_NULL_HANDLE);

    if (abort || result == vk::Result::eErrorOutOfDateKHR ||
        result == vk::Result::eSuboptimalKHR) {
        // If the swap chain turns out to be out of date when attempting to
        // acquire an image, then it is no longer possible to present to it.
        // Therefore we should immediately recreate the swap chain and try
        // again in the next drawFrame call.
        std::optional<uint32_t> o;
        return o;
    } else if (result != vk::Result::eSuccess) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Delay resetting the fence until after we know for sure we will be
    // submitting work with it. Thus, if we return early, the fence is still
    // signaled and vkWaitForFences wont deadlock the next time we use the
    // same fence object.
    inFlightFences[currentFrame]->reset();

    return imageIndex;
}

void CommandPool::submitCommandBuffer() {
    // The first three parameters specify which semaphores to wait on before
    // execution begins and in which stage(s) of the pipeline to wait. We
    // want to wait with writing colors to the image until it's available,
    // so we're specifying the stage of the graphics pipeline that writes to
    // the color attachment. That means that theoretically the
    // implementation can already start executing our vertex shader and such
    // while the image is not yet available. Each entry in the waitStages
    // array corresponds to the semaphore with the same index in
    // pWaitSemaphores.
    vk::SubmitInfo submitInfo{};
    submitInfo.sType = vk::StructureType::eSubmitInfo;

    vk::Semaphore waitSemaphores[] = {
        *imageAvailableSemaphores[currentFrame]->handle};
    vk::PipelineStageFlags waitStages[] = {
        vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // The next two parameters specify which command buffers to actually
    // submit for execution.
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &*commandBuffers[currentFrame];

    // The signalSemaphoreCount and pSignalSemaphores parameters specify
    // which semaphores to signal once the command buffer(s) have finished
    // execution.
    vk::Semaphore signalSemaphores[] = {
        *renderFinishedSemaphores[currentFrame]->handle};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // We can now submit the command buffer to the graphics queue using
    // vkQueueSubmit. The function takes an array of vk::SubmitInfo structures
    // as argument for efficiency when the workload is much larger. The last
    // parameter references an optional fence that will be signaled when the
    // command buffers finish execution. This allows us to know when it is
    // safe for the command buffer to be reused, thus we want to give it
    // inFlightFence. Now on the next frame, the CPU will wait for this
    // command buffer to finish executing before it records new commands
    // into it.
    device->graphicsQueue.submit({submitInfo},
                                 *inFlightFences[currentFrame]->handle);
}

void CommandPool::createCommandPool() {
    // You have to record all of the operations you want to perform in
    // command buffer objects. The advantage of this is that when we are
    // ready to tell the Vulkan what we want to do, all of the commands are
    // submitted together and Vulkan can more efficiently process the
    // commands since all of them are available together. In addition, this
    // allows command recording to happen in multiple threads if so desired.

    QueueFamilyIndices queueFamilyIndices =
        device->physical->findQueueFamilies();

    {
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;

        // - VK_COMMAND_POOL_CREATE_TRANSIENT_BIT: Hint that command buffers are
        // rerecorded with new commands very often (may change memory allocation
        // behavior)
        // - VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT: Allow command
        // buffers to be rerecorded individually, without this flag they all
        // have to be reset together

        // We will be recording a command buffer every frame, so we want to be
        // able to reset and rerecord over it.
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        commandPool = device->device.createCommandPool(poolInfo);
    }

    {
        // Memory transfer operations are executed using command buffers, just
        // like drawing commands. Therefore we must first allocate a temporary
        // command buffer. You may wish to create a separate command pool for
        // these kinds of short-lived buffers, because the implementation may be
        // able to apply memory allocation optimizations. You should use the
        // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT flag during command pool
        // generation in that case.
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
                         vk::CommandPoolCreateFlagBits::eTransient;
        poolInfo.queueFamilyIndex = queueFamilyIndices.transferFamily.value();

        commandPoolTransfer = device->device.createCommandPool(poolInfo);
    }

    {
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.sType = vk::StructureType::eCommandPoolCreateInfo;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        commandPoolRenderer = device->device.createCommandPool(poolInfo);
    }
}

void CommandPool::createCommandBuffers() {

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
    allocInfo.commandPool = *commandPool;

    // The level parameter specifies if the allocated command buffers are
    // primary or secondary command buffers.
    // - vk::CommandBufferLevel::ePrimary: Can be submitted to a queue for
    // execution, but cannot be called from other command buffers.
    // - VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly,
    // but can be called from primary command buffers.

    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    commandBuffers = device->device.allocateCommandBuffers(allocInfo);

    simpleCommandBuffers.reserve(commandBuffers.size());
    for (size_t i = 0; i < commandBuffers.size(); i++) {
        simpleCommandBuffers.push_back(*commandBuffers[i]);
    }
}

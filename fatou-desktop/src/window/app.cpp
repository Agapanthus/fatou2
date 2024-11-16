#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "app.h"

#include <iostream>
#include <cstdlib>

#ifdef WITH_GUI
#include "gui/cef/cefModule.h"
#endif

#include "console.h"

#include "sharedTexture.h"
shared_ptr<OnlineTexture> targetTexture;
std::mutex targetMutex;

std::unique_lock initialLock(targetMutex);

SafeQueue<InputEventData> inputEventQueue;

std::atomic<HWND> shared_hwnd;
std::condition_variable hwndReady;
std::mutex hwndReadyMutex;
std::unique_lock waitingForHwndReady(hwndReadyMutex);

std::atomic<bool> mainLoopRunning = true;
std::atomic<bool> jsConnected = false;

double targetZoomLevel = 1.;

std::atomic<GLFWwindow *> glfwWindow;

std::atomic<IRect> renderAreaRect = IRect({0, 0, 0, 0});

SafeQueue<string> presetLoader;

App::App() : win(make_shared<NativeWindow>("fatou", this)) {

    try {
        shared_hwnd = win->handle();
        waitingForHwndReady.unlock();

        instance = make_shared<VulkanInstance>(win->getRequiredExtensions());

        // The window surface needs to be created right after the instance
        // creation, because it can actually influence the physical device
        // selection. It should also be noted that window surfaces are an
        // entirely optional component in Vulkan, if you just need off-screen
        // rendering.
        win->createSurface(instance);

        device =
            make_shared<LogicalDevice>(PhysicalDevice::pickPhysicalDevice(win));
        swapChain = make_shared<SwapChain>(device);

        commandPool = make_shared<CommandPool>(device);

        compositor = make_shared<Compositor>(device, *device->transferQueue,
                                             swapChain, commandPool);

        std::array<uint8_t, 4> redPixel = {255, 0, 0, 255};
        loaderTexture = make_shared<Texture>(device, redPixel.data(), 1, 1,
                                             commandPool->transfer(),
                                             *device->transferQueue);
        loaderDescriptorPool = compositor->makeDP(loaderTexture->imageView());

#ifndef WITH_GUI
        const auto e = swapChain->extent();
        IRect r = {0, e.width, e.height, 0};
        renderAreaRect = r;
#endif

        updateGUITexture();

        recreateMandelPipe();

        initialLock.unlock();

    } catch (std::exception e) {
        fatalBox(e.what());
    }
}

void App::recreateMandelPipe() {
    mandel.reset();

    IRect r = renderAreaRect;
    int w = r.right - r.left;
    int h = r.bottom - r.top;

    if (w > 0 && h > 0) {
        Extent2D e;
        e.width = w;
        e.height = h;

        mandel = make_shared<Fractal_Mandel>(device, e, commandPool,
                                             swapChain->getPhases());
        mandel->makeDP(*compositor);
    }
}

void App::updateGUITexture() {
#ifdef WITH_GUI
    int w = swapChain->extent().width;
    int h = swapChain->extent().height;

    uint8_t *data = new uint8_t[w * h * 4];
    memset(data, 0, w * h * 4);
    cefTexture =
        make_shared<OnlineTexture>(device, commandPool->transfer(), w, h);
    cefTexture->update(data);
    delete[] data;
    targetTexture = cefTexture;

    cefDescriptorPool.reset();
    cefDescriptorPool = compositor->makeDP(cefTexture->imageView());

    if (CefModule::getInstance())
        CefModule::getInstance()->resize(w, h);

#else
    // update the render target size
    IRect r = {0, swapChain->extent().width, swapChain->extent().height, 0};
    renderAreaRect = r;
#endif
}

void App::recreateSwapChain() {
    // wait and do nothing while window is minimized
    win->waitWhileMinimized();
    device->waitIdle();

    // TODO: already locked by mainloop. Always?
    // std::unique_lock lk(targetMutex);

    compositor->setSwapchain(nullptr);

    // swapChain.reset();
    //  TODO: Do the recreate more elegantly
    swapChain = make_shared<SwapChain>(device, swapChain->handle());

    compositor->setSwapchain(swapChain);

    updateGUITexture();

    // lk.unlock();
}

void App::maybeRecreateRenderTexture() {
    IRect r = renderAreaRect;
    int w = r.right - r.left;
    int h = r.bottom - r.top;

    if (w == 0 || h == 0)
        return;
    if (mandel.get() && mandel->getExtent().width == w &&
        mandel->getExtent().height == h)
        return;

    device->waitIdle();

    // TODO: already locked by mainloop. Always?
    // std::unique_lock lk(targetMutex);

    // size has changed
    recreateMandelPipe();

    // lk.unlock();
}

void App::renderStep() {
    /*
    static auto startTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = startTime - std::chrono::seconds(1);
    auto currentTime = std::chrono::high_resolution_clock::now();

    if (mandel.get()) {
        if (currentTime - lastTime > std::chrono::seconds(1)) {
            vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = vk::StructureType::eCommandBufferAllocateInfo;
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandPool = commandPool->renderer();
            allocInfo.commandBufferCount = 1;

            vector<vk::raii::CommandBuffer> commandBuffers =
                device->device.allocateCommandBuffers(allocInfo);

            // SingleTimeCommandManager manager(commandPool->renderer(),
*device->device, *device->graphicsQueue);

            {
                CommandBufferRecorder rec(*commandBuffers[0]);

                lastTime = currentTime;
                mandel->renderStep(rec, *commandBuffers[0]);
            }

            vk::SubmitInfo submitInfo{};
            submitInfo.sType = vk::StructureType::eSubmitInfo;
            submitInfo.commandBufferCount = commandBuffers.size();
            submitInfo.pCommandBuffers = &*commandBuffers[0];

            device->graphicsQueue.submit(submitInfo, {});

            // TODO: to prevent data race with presentation; smart semaphore and
            // stuff are probably necessary here
            device->graphicsQueue.waitIdle();

        }
    }
    */
}

void App::recordCommandBuffer(vk::CommandBuffer commandBuffer,
                              uint32_t imageIndex) {

    static auto startTime = std::chrono::high_resolution_clock::now();
    static auto lastTime = startTime - std::chrono::seconds(1);
    auto currentTime = std::chrono::high_resolution_clock::now();
    // in seconds
    float time = std::chrono::duration<float, std::chrono::seconds::period>(
                     currentTime - startTime)
                     .count();

    CommandBufferRecorder rec(commandBuffer);

    if (mandel.get()) {
        // if (currentTime - lastTime > std::chrono::milliseconds(200))
        {
            lastTime = currentTime;
            mandel->renderStep(rec, commandBuffer, imageIndex);
        }
    }

    // TODO: to prevent data race with presentation; smart semaphore and
    // stuff are probably necessary here

    {
        RenderPassManager manager(rec, *swapChain, imageIndex);

        ///////////////////////////
        compositor->begin(commandBuffer);

        if (mandel.get()) {
            IRect r = renderAreaRect;
            mandel->present(commandBuffer, *compositor, r);
        }

        if (!mandel.get()) {
            compositor->setTransform(
                loaderDescriptorPool.get(),
                glm::rotate(
                    glm::scale(
                        glm::mat4(1.0f),
                        glm::vec3(.1f,
                                  .1f * float(swapChain->extent().width) /
                                      float(swapChain->extent().height),
                                  1.0f)),
                    sin(time) * glm::radians(10 * 30.0f),
                    glm::vec3(0.0f, 0.0f, 1.0f)));
            compositor->draw(commandBuffer, loaderDescriptorPool.get());
        }

#ifdef WITH_GUI
        compositor->setTransform(cefDescriptorPool.get(), glm::mat4(1.0f),
                                 true);
        compositor->draw(commandBuffer, cefDescriptorPool.get());
#endif
    }
}

void App::mainLoop() {
    try {
        std::chrono::duration frameTime =
            std::chrono::milliseconds(1000 / 61); // 61Hz
        auto nextFrame = std::chrono::system_clock::now();
        auto lastFrame = nextFrame - frameTime;

        while (!win->active()) {

            static auto fpsStart = std::chrono::high_resolution_clock::now();
            static int frameCounter = 0;
            auto currentTime = std::chrono::high_resolution_clock::now();

            frameCounter += 1;

            // in seconds
            float dt =
                std::chrono::duration<float, std::chrono::seconds::period>(
                    currentTime - fpsStart)
                    .count();
            const float refreshFPSEvery = 1.0;
            if (dt >= refreshFPSEvery) {
                float fps = frameCounter / dt;
                commitJS("setRenderParams", "{fps:" + jsStrD(fps) + "}");
                fpsStart = currentTime;
                frameCounter = 0;
            }

            win->poll();

            std::unique_lock lk(targetMutex);

            // - Wait for the previous frame to finish
            // - Acquire an image from the swap chain
            // - Record a command buffer which draws the scene onto that image
            // - Submit the recorded command buffer
            // - FPresent the swap chain image

            maybeRecreateRenderTexture();

            optional<uint32_t> imageIndex = commandPool->acquireNextImage(
                swapChain->swapChain, framebufferResized);
            if (!imageIndex.has_value()) {
                framebufferResized = false;
                recreateSwapChain();
                continue;
            }

            renderStep();

            commandPool->resetCommandBuffer();
            recordCommandBuffer(commandPool->currentBuffer(),
                                imageIndex.value());
            commandPool->submitCommandBuffer();
            commandPool->presentFrame(swapChain->swapChain, imageIndex.value());
            commandPool->swapBuffers();

            lk.unlock();

            std::this_thread::sleep_until(nextFrame);
            lastFrame = nextFrame;
            nextFrame += frameTime;
        }

        // wait for everything to shut down
        device->waitIdle();

    } catch (std::exception e) {
        fatalBox(e.what());
    }
}

App::~App() {
    targetTexture.reset();
    swapChain.reset();
}
#pragma once

#include "shader.h"
#include "nativeWindow.h"
#include "pingable.h"
#include "logicalDevice.h"
#include "swapChain.h"
#include "commandBuffer.h"
#include "renderPass.h"
#include "buffer.h"
#include "texture.h"
#include "fractal.h"
#include "compositor.h"

class App : public Pingable {
  public:
    App();
    ~App();

    void run() { mainLoop(); }

  private:
    shared_ptr<NativeWindow> win;
    shared_ptr<VulkanInstance> instance;
    shared_ptr<LogicalDevice> device;
    shared_ptr<SwapChain> swapChain;
    shared_ptr<CommandPool> commandPool;

    shared_ptr<DescriptorPool> loaderDescriptorPool;
    shared_ptr<DescriptorPool> cefDescriptorPool;

    shared_ptr<Compositor> compositor;

    shared_ptr<OnlineTexture> cefTexture;
    shared_ptr<Texture> loaderTexture;

    shared_ptr<Fractal_Mandel> mandel;

    bool framebufferResized = false;

    void ping() { framebufferResized = true; }

    void updateGUITexture();

  private:
    void recreateSwapChain();
    void recordCommandBuffer(vk::CommandBuffer commandBuffer,
                             uint32_t imageIndex);
    void mainLoop();
    
    void renderStep();
    void recreateMandelPipe();
    void maybeRecreateRenderTexture();
};

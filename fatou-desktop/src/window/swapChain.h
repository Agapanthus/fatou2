#pragma once

#include "logicalDevice.h"
#include "pipeline.h"
#include "shader.h"
#include "vulkanUtil.h"
#include "texture.h"

const path shaderPath = "shaders";

class SwapChain : private boost::noncopyable {
  public:
    SwapChain(shared_ptr<LogicalDevice> device,
              vk::SwapchainKHR oldSwapchain = nullptr)
        : device(device), swapChain(device->device.createSwapchainKHR(
                              createInfo(oldSwapchain))) {

        // Note, that you don't get vk::raii::Images here, but plain VkImages.
        // They are controlled by the swap chain, and you should not destroy
        // them.
        images = swapChain.getImages();

        createImageViews();

        // TODO: Separate pipeline!
        pipeline = make_shared<Pipeline<DescriptorSetLayout>>(
            device,
            make_shared<Shader>(device,
                                shaderPath / "playground" / "simple.vert",
                                ShaderType::VERTEX),
            make_shared<Shader>(device, shaderPath / "playground" / "pass.frag",
                                ShaderType::FRAGMENT),
            swapChainExtent, imageFormat,
            make_shared<DescriptorSetLayout>(device),
            vk::ImageLayout::ePresentSrcKHR);

        createFramebuffers();
    }

    ~SwapChain() {
        std::cout << "Destroy Swap chain..." << std::endl;
        framebuffers.clear();
        /* for (size_t i = 0; i < framebuffers.size(); i++) {
             vkDestroyFramebuffer(device->handle(), framebuffers[i],
                                  nullptr);
         }*/

        pipeline.reset();

        imageViews.clear();
        /* for (size_t i = 0; i < imageViews.size(); i++) {
             vkDestroyImageView(device->handle(), imageViews[i],
                                nullptr);
         }*/

        // vkDestroySwapchainKHR(device->handle(), swapChain, nullptr);
    }

    vk::SwapchainKHR handle() { return *swapChain; }

    Extent2D extent() { return swapChainExtent; }

    size_t getPhases() {
      return images.size();
    }

  private:
    Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
    vk::SwapchainCreateInfoKHR createInfo(vk::SwapchainKHR &oldSwapchain);
    void createSwapChainImage();
    void createImageViews();
    void createFramebuffers();

  protected:
    shared_ptr<LogicalDevice> device;
    Extent2D swapChainExtent;

  public:
    vk::raii::SwapchainKHR swapChain;

  protected:
    vk::Format imageFormat;

    vector<VkImage> images;
    vector<vk::raii::ImageView> imageViews;
    vector<vk::raii::Framebuffer> framebuffers;

    friend class RenderPassManager;

  public: // TODO: shouldn't be
    shared_ptr<Pipeline<DescriptorSetLayout>> pipeline;
};

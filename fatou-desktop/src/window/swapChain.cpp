#include "swapChain.h"
#include "vulkanUtil.h"

vk::SurfaceFormatKHR
chooseSwapSurfaceFormat(const vector<vk::SurfaceFormatKHR> &availableFormats) {
    // i.e., color depth

    // prefer VK_FORMAT_B8G8R8A8_SRGB
    for (const auto &availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
            availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return availableFormat;
        }
    }

    // otherwise, just take anything
    return availableFormats[0];
}

vk::PresentModeKHR
chooseSwapPresentMode(const vector<vk::PresentModeKHR> &availablePresentModes) {
    /*
    VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are
    transferred to the screen right away, which may result in tearing.

    VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes
    an image from the front of the queue when the display is refreshed and the
    program inserts rendered images at the back of the queue. If the queue is
    full then the program has to wait. This is most similar to vertical sync as
    found in modern games. The moment that the display is refreshed is known as
    "vertical blank".

    VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous
    one if the application is late and the queue was empty at the last vertical
    blank. Instead of waiting for the next vertical blank, the image is
    transferred right away when it finally arrives. This may result in visible
    tearing.

    VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode.
    Instead of blocking the application when the queue is full, the images that
    are already queued are simply replaced with the newer ones. This mode can be
    used to render frames as fast as possible while still avoiding tearing,
    resulting in fewer latency issues than standard vertical sync. This is
    commonly known as "triple buffering", although the existence of three
    buffers alone does not necessarily mean that the framerate is unlocked.
    */

    /*
    I personally think that VK_PRESENT_MODE_MAILBOX_KHR is a very nice trade-off
    if energy usage is not a concern. It allows us to avoid tearing while still
    maintaining a fairly low latency by rendering new images that are as
    up-to-date as possible right until the vertical blank. On mobile devices,
    where energy usage is more important, you will probably want to use
    VK_PRESENT_MODE_FIFO_KHR instead
    */

    for (const auto &availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

Extent2D
SwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) {
    /*
    GLFW uses two units when measuring sizes: pixels and screen coordinates. For
    example, the resolution {WIDTH, HEIGHT} that we specified earlier when
    creating the window is measured in screen coordinates. But Vulkan works with
    pixels, so the swap chain extent must be specified in pixels as well.
    Unfortunately, if you are using a high DPI display (like Apple's Retina
    display), screen coordinates don't correspond to pixels. Instead, due to the
    higher pixel density, the resolution of the window in pixel will be larger
    than the resolution in screen coordinates. So if Vulkan doesn't fix the swap
    extent for us, we can't just use the original {WIDTH, HEIGHT}. Instead, we
    must use glfwGetFramebufferSize to query the resolution of the window in
    pixel before matching it against the minimum and maximum image extent.
    */

    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        Extent2D actualExtent = device->physical->getWindow()->getExtent();
        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

vk::SwapchainCreateInfoKHR
SwapChain::createInfo(vk::SwapchainKHR &oldSwapchain) {
    SwapChainSupportDetails swapChainSupport =
        device->physical->querySwapChainSupport();

    vk::SurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode =
        chooseSwapPresentMode(swapChainSupport.presentModes);
    Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // simply sticking to this minimum means that we may sometimes have to wait
    // on the driver to complete internal operations before we can acquire
    // another image to render to. Therefore it is recommended to request at
    // least one more image than the minimum:
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // We should also make sure to not exceed the maximum number of images while
    // doing this, where 0 is a special value that means that there is no
    // maximum:
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo{};
    createInfo.sType = vk::StructureType::eSwapchainCreateInfoKHR;
    createInfo.surface = device->physical->getWindow()->getSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    // This is always 1 unless you are developing a stereoscopic 3D application.
    createInfo.imageArrayLayers = 1;
    // The imageUsage bit field specifies what kind of operations we'll use the
    // images in the swap chain for. In this tutorial we're going to render
    // directly to them, which means that they're used as color attachment. It
    // is also possible that you'll render images to a separate image first to
    // perform operations like post-processing. In that case you may use a value
    // like vk::ImageUsageFlagBits::eTransferDst instead and use a memory
    // operation to transfer the rendered image to a swap chain image.
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    QueueFamilyIndices indices = device->physical->findQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        // vk::SharingMode::eConcurrent: Resources can be used across multiple
        // queue families without explicit ownership transfers.
        std::cout << "Warning: Using concurrent queues. Performance might be "
                     "suboptimal."
                  << std::endl;
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        // vk::SharingMode::eExclusive: a resource is owned by one queue family
        // at a time and ownership must be explicitly transferred before using
        // it in another queue family. This option offers the best performance.
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    // We can specify that a certain transform should be applied to images in
    // the swap chain if it is supported (supportedTransforms in capabilities),
    // like a 90 degree clockwise rotation or horizontal flip. To specify that
    // you do not want any transformation, simply specify the current
    // transformation.
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    // The compositeAlpha field specifies if the alpha channel should be used
    // for blending with other windows in the window system. You'll almost
    // always want to simply ignore the alpha channel, hence
    // VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

    createInfo.presentMode = presentMode;

    // If the clipped member is set to VK_TRUE then that means that we don't
    // care about the color of pixels that are obscured, for example because
    // another window is in front of them. Unless you really need to be able to
    // read these pixels back and get predictable results, you'll get the best
    // performance by enabling clipping.
    createInfo.clipped = VK_TRUE;

    // With Vulkan it's possible that your swap chain becomes invalid or
    // unoptimized while your application is running, for example because the
    // window was resized. In that case the swap chain actually needs to be
    // recreated from scratch and a reference to the old one must be specified
    // in this field.
    // TODO:
    createInfo.oldSwapchain = oldSwapchain;

    imageFormat = (vk::Format)surfaceFormat.format;
    this->swapChainExtent = Extent2D(extent.width, extent.height);

    return createInfo;
}

void SwapChain::createImageViews() {
    imageViews.clear();
    for (size_t i = 0; i < images.size(); i++) {
        imageViews.push_back(createImageView(
            device->device, images[i], imageFormat,
            vk::ImageAspectFlagBits::eColor));
    }
}

void SwapChain::createFramebuffers() {
    framebuffers.clear();

    for (size_t i = 0; i < imageViews.size(); i++) {
        vk::ImageView attachments[] = {*imageViews[i]};

        vk::FramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = vk::StructureType::eFramebufferCreateInfo;

        // You can only use a framebuffer with the render passes that it is
        // compatible with, which roughly means that they use the same number
        // and type of attachments.
        framebufferInfo.renderPass = pipeline->getRenderPass();

        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;

        // Our swap chain images are single images, so the number of layers
        // is 1.
        framebufferInfo.layers = 1;

        framebuffers.push_back(
            device->device.createFramebuffer(framebufferInfo));
    }
}

#pragma once
#include "buffer.h"
#include "vulkanUtil.h"

void createImage(const LogicalDevice *device, int w, int h, vk::Format format,
                 vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                 vk::MemoryPropertyFlags properties, vk::raii::Image &image,
                 vk::raii::DeviceMemory &imageMemory);

void transitionImageLayout(const vk::raii::Device &device,
                           vk::CommandPool commandPool, vk::Queue transferQueue,
                           vk::Image image, vk::Format format,
                           vk::ImageLayout oldLayout,
                           vk::ImageLayout newLayout);

uint8_t *loadFile(const string &path, int &w, int &h);

inline vk::raii::ImageView createImageView(const vk::raii::Device &device,
                                           vk::Image image, vk::Format format,
                                           vk::ImageAspectFlags aspectMask) {
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.sType = vk::StructureType::eImageViewCreateInfo;
    viewInfo.image = image;

    // allows you to treat images as 1D textures, 2D textures, 3D textures
    // and cube maps.
    viewInfo.viewType = vk::ImageViewType::e2D;
    viewInfo.format = format;

    // The components field allows you to swizzle the color channels around.
    // For example, you can map all of the channels to the red channel for a
    // monochrome texture. You can also map constant values of 0 and 1 to a
    // channel. In our case we'll stick to the default mapping.
    viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
    viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
    viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
    viewInfo.components.a = vk::ComponentSwizzle::eIdentity;

    // The subresourceRange field describes what the image's purpose is and
    // which part of the image should be accessed. Our images will be used
    // as color targets without any mipmapping levels or multiple layers. If
    // you were working on a stereographic 3D application, then you would
    // create a swap chain with multiple layers. You could then create
    // multiple image views for each image representing the views for the
    // left and right eyes by accessing different layers.
    viewInfo.subresourceRange.aspectMask = aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    return device.createImageView(viewInfo);
}

class Texture : private boost::noncopyable {
  public:
    Texture(shared_ptr<LogicalDevice> device, const uint8_t *data, int w, int h,
            vk::CommandPool commandPool, vk::Queue transferQueue)
        : device(device), w(w), h(h), textureImageView(0), textureImage(0),
          aspectMask(vk::ImageAspectFlagBits::eColor), textureImageMemory(0) {
        createTextureImage(data, commandPool, transferQueue);
        textureImageView =
            createImageView(device->device, *textureImage,
                            vk::Format::eR8G8B8A8Srgb, aspectMask);
    }

    static shared_ptr<Texture> fromFile(shared_ptr<LogicalDevice> device,
                                        const string &path,
                                        vk::CommandPool commandPool,
                                        vk::Queue transferQueue);

    ~Texture() {
        //   vkDestroyImageView(device->handle(), textureImageView, nullptr);
        //   vkDestroyImage(device->handle(), textureImage, nullptr);
        //   vkFreeMemory(device->handle(), textureImageMemory, nullptr);
    }

    vk::ImageView imageView() const { return *textureImageView; }

  private:
    void createTextureImage(const uint8_t *data, vk::CommandPool commandPool,
                            vk::Queue transferQueue);

  private:
    int w;
    int h;
    const vk::ImageAspectFlags aspectMask;

    const shared_ptr<LogicalDevice> device;
    vk::raii::ImageView textureImageView;
    vk::raii::Image textureImage;
    vk::raii::DeviceMemory textureImageMemory;
};

class OnlineTexture : private boost::noncopyable {
    // TODO: https://stackoverflow.com/a/40575629/6144727
  public:
    OnlineTexture(
        shared_ptr<LogicalDevice> device, vk::CommandPool commandPool, int w,
        int h, vk::ImageUsageFlags moreFlags = {},
        vk::Format format = vk::Format::eR8G8B8A8Srgb,
        vk::ImageAspectFlags aspectMask = vk::ImageAspectFlagBits::eColor)
        : device(device), w(w), h(h), commandPool(commandPool), format(format),
          aspectMask(aspectMask), transferQueue(*device->transferQueue),
          textureImageView(0), textureImage(0), textureImageMemory(0) {
        createTextureImage(commandPool, transferQueue, moreFlags);
        textureImageView =
            createImageView(device->device, *textureImage, format, aspectMask);
    }

    ~OnlineTexture() {
        //  vkDestroyImageView(device->handle(), textureImageView, nullptr);
        //  vkDestroyImage(device->handle(), textureImage, nullptr);
        //  vkFreeMemory(device->handle(), textureImageMemory, nullptr);
    }

    vk::ImageView imageView() const { return *textureImageView; }
    vk::Image image() const { return *textureImage; }

    void update(const uint8_t *data);

    void transitionToRead();
    void transitionTo(vk::ImageLayout l);

    vk::ImageLayout imageLayout() { return layout; }

  private:
    void createTextureImage(vk::CommandPool commandPool,
                            vk::Queue transferQueue,
                            vk::ImageUsageFlags moreFlags = {});

  public:
    const int w;
    const int h;
    const vk::Format format;
    const vk::ImageAspectFlags aspectMask;

  private:
    // Maybe better not have them as members and use them freshly on update?
    const vk::CommandPool commandPool;
    const vk::Queue transferQueue;

    vk::ImageLayout layout;

    const shared_ptr<LogicalDevice> device;
    vk::raii::ImageView textureImageView;
    vk::raii::Image textureImage;
    vk::raii::DeviceMemory textureImageMemory;

    shared_ptr<Buffer> buf;
};

class Sampler : private boost::noncopyable {
  public:
    Sampler(shared_ptr<LogicalDevice> device)
        : device(device), textureSampler(nullptr) {
        createTextureSampler();
    }
    ~Sampler() { // vkDestroySampler(device->handle(), textureSampler, nullptr);
    }
    vk::Sampler handle() const { return *textureSampler; }

  private:
    void createTextureSampler();

  private:
    const shared_ptr<LogicalDevice> device;
    vk::raii::Sampler textureSampler;
};
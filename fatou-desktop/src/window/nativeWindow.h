#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "pingable.h"
#include "vulkanInstance.h"

double thisMonitorZoom(HWND hWnd);
HWND getNativeFromGLFW(GLFWwindow *window);
void getMonitorDPI();

class Navigator {
  public:
    Navigator() {
        z = 4.0;
        x = 0.;
        y = 0.;
        mx0 = -1;
        my0 = -1;
        updateXYZ();
    }

    void onScroll(int x, int y, double dx, double dy);
    void onDown(int x, int y);
    void onMove(int x, int y);
    void onRelease(int x, int y);
    void onLeave(int x, int y);

    void updateXYZ();

    void getAdjustedPos(double &ax, double &ay);

  public:
    double z;
    double x;
    double y;

    double x0;
    double y0;
    int mx0;
    int my0;

};

extern Navigator navi;

class NativeWindow : public Pingable,
                     public std::enable_shared_from_this<NativeWindow>,
                     private boost::noncopyable {
  public:
    NativeWindow(const string &title, Pingable *notifyChanged)
        : notifyChanged(notifyChanged), title(title) {
        initWindow();
    }

    void setKeyCallback(GLFWkeyfun key_callback) {
        glfwSetKeyCallback(window, key_callback);
    }

    ~NativeWindow();

    void ping() { notifyChanged->ping(); }

    vector<const char *> getRequiredExtensions() const;

    void getFrameBufferSize(int &width, int &height) {
        glfwGetFramebufferSize(window, &width, &height);
    }

    HWND handle() const { return getNativeFromGLFW(window); }

    void createSurface(shared_ptr<VulkanInstance> instance);

    vk::SurfaceKHR getSurface() { return surface; }

    void waitEvents() { glfwWaitEvents(); }

    bool active() { return glfwWindowShouldClose(window); }
    void poll() { glfwPollEvents(); }

    shared_ptr<VulkanInstance> getInstance() { return instance; }

    Extent2D getExtent() {
        int width, height;
        this->getFrameBufferSize(width, height);

        Extent2D actualExtent = {static_cast<uint32_t>(width),
                                   static_cast<uint32_t>(height)};
        return actualExtent;
    }

    void waitWhileMinimized() {
        int width = 0, height = 0;
        getFrameBufferSize(width, height);
        while (width == 0 || height == 0) {
            getFrameBufferSize(width, height);
            waitEvents();
        }
    }

  private:
    GLFWwindow *window;
    vk::SurfaceKHR surface;

    shared_ptr<VulkanInstance> instance;

    const string &title;
    Pingable *notifyChanged;

    const uint32_t WIDTH = 1535; //1536;
    const uint32_t HEIGHT = 1024;

  private:
    void initWindow();
};
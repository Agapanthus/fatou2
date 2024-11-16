#pragma once

#include "texture.h"
#include <condition_variable>
#include "safeQueue.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

extern std::mutex targetMutex;
extern shared_ptr<OnlineTexture> targetTexture;

enum InputEventDataType {
    MY_KEY_EVENT,
    MY_CHAR_EVENT,
    MY_MOUSE_MOVE_EVENT,
    MY_MOUSE_WHEEL_EVENT,
    MY_MOUSE_CLICK_EVENT
};

#ifdef WITH_GUI
#include <include/cef_client.h>
#endif

struct InputEventData {
    InputEventDataType type;
#ifdef WITH_GUI
    CefKeyEvent keyEvent;
    CefMouseEvent mouseEvent;
    CefBrowserHost::MouseButtonType mouseButton;
#endif

    int x;
    int y;
    int dx;
    int dy;
    bool mouseLeave;
    bool mouseUp;
    int clickCount;
};
extern SafeQueue<InputEventData> inputEventQueue;

extern std::atomic<HWND> shared_hwnd;
extern std::mutex hwndReadyMutex;
extern std::atomic<bool> mainLoopRunning;

extern std::atomic<bool> jsConnected;

struct IRect {
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
    uint32_t left;
};
extern std::atomic<IRect> renderAreaRect;

extern double targetZoomLevel;

extern std::atomic<GLFWwindow *> glfwWindow;


extern SafeQueue<string> presetLoader;
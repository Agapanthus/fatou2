#include "nativeWindow.h"

#include "sharedTexture.h"

#include "../gui/cef/js.h"

Navigator navi;

void Navigator::updateXYZ() {
    commitJS("setZ", jsStrD(z));
    commitJS("setX", jsStrD(x));
    commitJS("setY", jsStrD(y));
}

void Navigator::onScroll(int x, int y, double dx, double dy) {
    IRect r = renderAreaRect;
    if (x >= r.left && x <= r.right && y >= r.top && y <= r.bottom) {
        double dz = dx + dy;
        double de = dz / 25.0;

        if (de > 0) {
            z /= (1 + de);
        } else if (de < 0) {
            z *= (1 - de);
        }

        commitJS("setZ", jsStrD(z));
    }
}

void Navigator::getAdjustedPos(double &ax, double &ay) {
    ax = -x - z * 0.5;
    ay = y - z * 0.5;
}

void Navigator::onDown(int x, int y) {
    IRect r = renderAreaRect;
    if (x >= r.left && x <= r.right && y >= r.top && y <= r.bottom) {
        mx0 = x;
        my0 = y;
        x0 = this->x;
        y0 = this->y;
    }
}
void Navigator::onMove(int mx, int my) {
    if (mx0 >= 0 && my0 >= 0) {
        IRect r = renderAreaRect;
        int dx = mx - mx0;
        int dy = my - my0;

        int iw = r.right - r.left;
        int ih = r.bottom - r.top;
        float ax = std::max(1.0f, ih / float(iw));
        float ay = std::max(1.0f, iw / float(ih));

        x = x0 + dx * z / (iw * ax);
        y = y0 - dy * z / (ih * ay);

        commitJS("setX", jsStrD(x));
        commitJS("setY", jsStrD(y));
    }
}

void Navigator::onRelease(int x, int y) {
    mx0 = -1;
    my0 = -1;
}
void Navigator::onLeave(int x, int y) {
    mx0 = -1;
    my0 = -1;
}

int getEventModifiers(int mods) {
    int modifiers = 0;

#ifdef WITH_GUI
    if (mods & GLFW_MOD_SHIFT) {
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    }
    if (mods & GLFW_MOD_CONTROL) {
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    }
    if (mods & GLFW_MOD_ALT) {
        modifiers |= EVENTFLAG_ALT_DOWN;
    }
    if (mods & GLFW_MOD_CAPS_LOCK) {
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
    }
    if (mods & GLFW_MOD_NUM_LOCK) {
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;
    }
    /*

  if (wparam & MK_LBUTTON)
    modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
  if (wparam & MK_MBUTTON)
    modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
  if (wparam & MK_RBUTTON)
    modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
    */
#endif

    return modifiers;
}

int getEventModifiersAlways(GLFWwindow *window) {
    int modifiers = 0;

#ifdef WITH_GUI
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS) {
        modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
    }

    state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
    if (state == GLFW_PRESS) {
        modifiers |= EVENTFLAG_MIDDLE_MOUSE_BUTTON;
    }

    state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
    if (state == GLFW_PRESS) {
        modifiers |= EVENTFLAG_RIGHT_MOUSE_BUTTON;
    }

    if (GetKeyState(VK_SHIFT) & 0x8000)
        modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        modifiers |= EVENTFLAG_CONTROL_DOWN;
    if (GetKeyState(VK_MENU) & 0x8000)
        modifiers |= EVENTFLAG_ALT_DOWN;
    if (GetKeyState(VK_CAPITAL) & 1)
        modifiers |= EVENTFLAG_CAPS_LOCK_ON;
    if (GetKeyState(VK_NUMLOCK) & 1)
        modifiers |= EVENTFLAG_NUM_LOCK_ON;
#endif

    return modifiers;
}

void createKeyevent(int key, int action, int mods) {
#ifdef WITH_GUI
    InputEventData data;
    data.type = MY_KEY_EVENT;

    data.keyEvent.windows_key_code = key;
    data.keyEvent.native_key_code = 0; // key;

    data.keyEvent.is_system_key = key == GLFW_MOD_SUPER;

    switch (action) {
    case GLFW_REPEAT:
    case GLFW_PRESS:
        data.keyEvent.type = KEYEVENT_RAWKEYDOWN;
        // data.keyEvent.type = KEYEVENT_KEYDOWN;
        break;

    case GLFW_RELEASE:
        data.keyEvent.type = KEYEVENT_KEYUP;
        break;

    case 100:
    default:
        data.keyEvent.type = KEYEVENT_CHAR;
    }

    /*
     if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
         event.type = KEYEVENT_RAWKEYDOWN;
     else if (message == WM_KEYUP || message == WM_SYSKEYUP)
         event.type = KEYEVENT_KEYUP;
     else
        event.type = KEYEVENT_CHAR;
    */

    data.keyEvent.modifiers =
        getEventModifiers(mods); // GetCefKeyboardModifiers(wParam, lParam);

    inputEventQueue.enqueue(data);
#endif
}

int glfwToWin(int key) {
    if (key < 0)
        return 0;
    if (GLFW_KEY_A <= key && key <= GLFW_KEY_Z) {
        return 0x41 + (key - GLFW_KEY_A);
    }
    if (GLFW_KEY_F1 <= key && key <= GLFW_KEY_F25) {
        return VK_F1 + (key - GLFW_KEY_F1);
    }
    if (GLFW_KEY_0 <= key && key <= GLFW_KEY_9) {
        return 0x30 + (key - GLFW_KEY_0);
    }
    if (GLFW_KEY_KP_0 <= key && key <= GLFW_KEY_KP_9) {
        return VK_NUMPAD0 + (key - GLFW_KEY_KP_0);
    }
    switch (key) {

    case GLFW_KEY_SPACE:
        return VK_SPACE;

    case GLFW_KEY_APOSTROPHE:
        return VK_OEM_7; // strange

    case GLFW_KEY_COMMA:
        return VK_OEM_COMMA;

    case GLFW_KEY_MINUS:
        return VK_OEM_MINUS;

    case GLFW_KEY_PERIOD:
        return VK_OEM_PERIOD;

    case GLFW_KEY_SLASH:
        return VK_OEM_2;

    case GLFW_KEY_SEMICOLON:
        return 186;

    case GLFW_KEY_EQUAL:
        return 187;

    case GLFW_KEY_LEFT_BRACKET:
        return 0; // Not supported

    case GLFW_KEY_BACKSLASH:
        return 220;

    case GLFW_KEY_RIGHT_BRACKET:
        return 0; // not supported

    case GLFW_KEY_GRAVE_ACCENT:
        return 192;

    case GLFW_KEY_WORLD_1:
        return 0; // not supported

    case GLFW_KEY_WORLD_2:
        return 0; // not supported

    case GLFW_KEY_ESCAPE:
        return VK_ESCAPE;

    case GLFW_KEY_ENTER:
        return VK_RETURN;

    case GLFW_KEY_TAB:
        return VK_TAB;

    case GLFW_KEY_BACKSPACE:
        return VK_BACK;

    case GLFW_KEY_INSERT:
        return VK_INSERT;

    case GLFW_KEY_DELETE:
        return VK_DELETE;

    case GLFW_KEY_RIGHT:
        return VK_RIGHT;

    case GLFW_KEY_LEFT:
        return VK_LEFT;

    case GLFW_KEY_DOWN:
        return VK_DOWN;

    case GLFW_KEY_UP:
        return VK_UP;

    case GLFW_KEY_PAGE_UP:
        return VK_PRIOR;

    case GLFW_KEY_PAGE_DOWN:
        return VK_NEXT;

    case GLFW_KEY_HOME:
        return VK_HOME;

    case GLFW_KEY_END:
        return VK_END;

    case GLFW_KEY_CAPS_LOCK:
        return VK_CAPITAL;

    case GLFW_KEY_SCROLL_LOCK:
        return VK_SCROLL;

    case GLFW_KEY_NUM_LOCK:
        return VK_NUMLOCK;

    case GLFW_KEY_PRINT_SCREEN:
        return VK_PRINT;

    case GLFW_KEY_PAUSE:
        return VK_PAUSE;

    case GLFW_KEY_KP_DECIMAL:
        return VK_DECIMAL;

    case GLFW_KEY_KP_DIVIDE:
        return VK_DIVIDE;

    case GLFW_KEY_KP_MULTIPLY:
        return VK_MULTIPLY;

    case GLFW_KEY_KP_SUBTRACT:
        return VK_SUBTRACT;

    case GLFW_KEY_KP_ADD:
        return VK_ADD;

    case GLFW_KEY_KP_ENTER:
        return VK_RETURN;

    case GLFW_KEY_KP_EQUAL:
        return 187;

    case GLFW_KEY_LEFT_SHIFT:
        return VK_LSHIFT;

    case GLFW_KEY_LEFT_CONTROL:
        return VK_LCONTROL;

    case GLFW_KEY_LEFT_ALT:
        return VK_LMENU;

    case GLFW_KEY_LEFT_SUPER:
        return VK_LWIN;

    case GLFW_KEY_RIGHT_SHIFT:
        return VK_RSHIFT;

    case GLFW_KEY_RIGHT_CONTROL:
        return VK_RCONTROL;

    case GLFW_KEY_RIGHT_ALT:
        return VK_RMENU;

    case GLFW_KEY_RIGHT_SUPER:
        return VK_RWIN;

    case GLFW_KEY_MENU:
        return VK_MENU;
    }

    return 0;
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
    createKeyevent(glfwToWin(key), action, mods);
}

void charCallback(GLFWwindow *window, unsigned int codepoint) {
    createKeyevent(codepoint, 100, 0);
}

void fillMouseEvent(GLFWwindow *window, InputEventData &data) {
    double xpos;
    double ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    data.x = xpos;
    data.y = ypos;

#ifdef WITH_GUI
    data.mouseEvent.x = xpos;
    data.mouseEvent.y = ypos;
    data.mouseEvent.modifiers = getEventModifiersAlways(window);
    data.mouseLeave = false;
#endif
}

void cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    InputEventData data;
    data.type = MY_MOUSE_MOVE_EVENT;
    fillMouseEvent(window, data);
    inputEventQueue.enqueue(data);

    navi.onMove(data.x, data.y);
}

void cursorLeave(GLFWwindow *window, int entered) {

    if (entered) {
        // The cursor entered the content area of the window
    } else {
        // The cursor left the content area of the window
        InputEventData data;
        data.type = MY_MOUSE_MOVE_EVENT;
        fillMouseEvent(window, data);
        data.mouseLeave = true;
        inputEventQueue.enqueue(data);

        navi.onLeave(data.x, data.y);
    }
}

void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
    double scale = 100.0;
    InputEventData data;
    data.type = MY_MOUSE_WHEEL_EVENT;
    fillMouseEvent(window, data);
    data.dx = xoffset * scale;
    data.dy = yoffset * scale;
    navi.onScroll(data.x, data.y, xoffset, yoffset);

    // apparently there is a bug when it is too small?
    if (data.dx < -1 || data.dx > 1 || data.dy > 1 || data.dy < -1)
        inputEventQueue.enqueue(data);
}

void clickCallback(GLFWwindow *window, int button, int action, int mods) {
    InputEventData data;
    data.type = MY_MOUSE_CLICK_EVENT;
    fillMouseEvent(window, data);

#ifdef WITH_GUI
    switch (button) {
    case GLFW_MOUSE_BUTTON_MIDDLE:
        data.mouseButton = cef_mouse_button_type_t::MBT_MIDDLE;
    case GLFW_MOUSE_BUTTON_RIGHT:
        data.mouseButton = cef_mouse_button_type_t::MBT_RIGHT;
    default:
    case GLFW_MOUSE_BUTTON_LEFT:
        data.mouseButton = cef_mouse_button_type_t::MBT_LEFT;
    }
#endif

    data.mouseUp = action != GLFW_PRESS;
    data.clickCount = 1;

    if (action == GLFW_PRESS) {
        navi.onDown(data.x, data.y);
    } else {
        navi.onRelease(data.x, data.y);
    }

    // crappy double click detection https://github.com/glfw/glfw/issues/462
    // TODO: Does this work?
    // See here on how to make this properly
    // https://github.com/Mojang/cef/blob/master/tests/cefclient/browser/osr_window_win.cc
    if (action == GLFW_RELEASE) {
        static auto before = std::chrono::system_clock::now();
        auto now = std::chrono::system_clock::now();
        double diff_ms =
            std::chrono::duration<double, std::milli>(now - before).count();
        before = now;
        if (diff_ms > 10 && diff_ms < 500) {
            data.clickCount = 2;
            // std::cout << "double click" << std::endl;
        }
    }
    inputEventQueue.enqueue(data);
}

void closeCallback(GLFWwindow *window) {
    // if (!timeToClose) glfwSetWindowShouldClose(window, GLFW_FALSE);
    mainLoopRunning = false;
}

void refreshZoom(GLFWwindow *window) {
    double scale = thisMonitorZoom(getNativeFromGLFW(window));

    // "Each zoom level increases the zoom by 20%. So you get 100%, 120%, 144%,
    // and so on."
    double newLevel = log(scale) / log(1.2);

    if (newLevel != targetZoomLevel) {
        std::cout << "Zoom changed to " << newLevel << std::endl;
    }
    targetZoomLevel = newLevel;
}

void posCallback(GLFWwindow *window, int xpos, int ypos) {
    refreshZoom(window);
}

void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto app =
        reinterpret_cast<NativeWindow *>(glfwGetWindowUserPointer(window));
    app->ping();
}

void NativeWindow::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, title.c_str(), nullptr, nullptr);
    glfwWindow = window;
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCharCallback(window, charCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetCursorEnterCallback(window, cursorLeave);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetMouseButtonCallback(window, clickCallback);
    glfwSetWindowCloseCallback(window, closeCallback);
    glfwSetWindowPosCallback(window, posCallback);

    refreshZoom(window);
    getMonitorDPI();
}

vector<const char *> NativeWindow::getRequiredExtensions() const {

    // add extensions requireed by GLFW
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    vector<const char *> extensions(glfwExtensions,
                                    glfwExtensions + glfwExtensionCount);

    return extensions;
}

void NativeWindow::createSurface(shared_ptr<VulkanInstance> instance) {
    this->instance = instance;
    VkSurfaceKHR tmpSurface = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(instance->get(), window, nullptr,
                                &tmpSurface) != VK_SUCCESS) {
        throw runtime_error("failed to create window surface!");
    }
    surface = vk::SurfaceKHR(tmpSurface);
}

NativeWindow::~NativeWindow() {
    std::cout << "Destroy native window" << std::endl;

    vkDestroySurfaceKHR(instance->get(), surface, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

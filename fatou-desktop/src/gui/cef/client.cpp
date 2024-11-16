#include "client.h"

#include "../../window/sharedTexture.h"

#include "resourceProvider_local.h"
#include "resourceProvider_block.h"
#include "js.h"

// Add example Providers to the CefResourceManager.
void SetupResourceManager(CefRefPtr<CefResourceManager> resource_manager) {
    if (!CefCurrentlyOn(TID_IO)) {
        // Execute on the browser IO thread.
        CefPostTask(TID_IO,
                    base::BindOnce(SetupResourceManager, resource_manager));
        return;
    }

    const std::string &localOrigin = "https://app";

    // Add the Provider for dumping request contents.
    resource_manager->AddProvider(
        new ResourceProvider_block(boost::regex{"((https:\\/\\/app))(\\/.*)?"}),
        0, string("block"));

    resource_manager->AddProvider(new ResourceProvider_local(localOrigin), 100,
                                  string("local"));
}

BrowserClient::BrowserClient(RenderHandler *renderHandler)
    : m_renderHandler(renderHandler) {
    resource_manager_ = new CefResourceManager();
    SetupResourceManager(resource_manager_);
}

struct glfw_cursor_layout {
    void *next;
    HCURSOR handle;
};

bool BrowserClient::OnCursorChange(CefRefPtr<CefBrowser> browser,
                                   CefCursorHandle cursor,
                                   cef_cursor_type_t type,
                                   const CefCursorInfo &custom_cursor_info) {
    if (!::IsWindow(shared_hwnd))
        return false;

    // Change the plugin window's cursor.
    //  SetClassLongPtr(shared_hwnd, GCLP_HCURSOR,
    //                  static_cast<LONG>(reinterpret_cast<LONG_PTR>(cursor)));
    //  SetCursor(cursor);

    // https://github.com/glfw/glfw/issues/768
    GLFWcursor *glfwCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    glfw_cursor_layout *p = (glfw_cursor_layout *)glfwCursor;
    DestroyCursor(p->handle);
    p->handle = cursor;
    glfwSetCursor(glfwWindow, glfwCursor);

    return true;
}

bool BrowserClient::OnProcessMessageReceived(
    CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
    // Check the message name.
    const std::string &message_name = message->GetName();
    if (message_name == "guiReady") {
        // send everything that waited so far
        jsConnected = true;
        setJS("connected", true);
        std::cout << "JS connected" << std::endl;
        return true;
    } else if (message_name == "renderAreaResize") {
        const auto ag = message->GetArgumentList();
        std::cout << "render area resized " << ag->GetInt(3) << std::endl;
        renderAreaRect =
            IRect({uint32_t(ag->GetInt(0)), uint32_t(ag->GetInt(1)),
                   uint32_t(ag->GetInt(2)), uint32_t(ag->GetInt(3))});
        return true;
    } else if (message_name == "loadPreset") {
        const auto ag = message->GetArgumentList();
        const string st = ag->GetString(0);
        std::cout << "load preset " << st << std::endl;

        presetLoader.enqueue(st);

        return true;
    }

    return false;
}
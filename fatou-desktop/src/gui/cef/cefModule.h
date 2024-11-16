#pragma once

#define NOMINMAX
#include "windows.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>

#include "renderHandler.h"
#include "client.h"

// based on
// https://github.com/daktronics/cef-mixer/blob/master/src/web_layer.cpp

// Lifetime management for CEF components.
// Manages the CEF message loop and CefInitialize/CefShutdown
class CefModule {
  public:
    CefModule(HINSTANCE mod) : module_(mod) { ready_ = false; }

    static void startup(HINSTANCE mod);
    static void shutdown();

    void resize(int w, int h) {
        renderHandler->resize(w, h);
        browser->GetHost()->WasResized();
        //browser->GetHost()->WasHidden(false);
    }

    static CefModule *getInstance() { return instance_.get(); }

  private:
    //
    // simple CefTask we'll post to our message-pump
    // thread to stop it (required to break out of CefRunMessageLoop)
    //
    class QuitTask : public CefTask {
      public:
        QuitTask() {}
        void Execute() override { CefQuitMessageLoop(); }
        IMPLEMENT_REFCOUNTING(QuitTask);
    };

    void message_loop();
    void loop_loop();
    void loop_setupRenderer();
    void loop_init();
    void loop_clean();

  private:
    std::condition_variable signal_;
    std::atomic_bool ready_;
    std::mutex lock_;
    HINSTANCE const module_;
    shared_ptr<std::thread> thread_;
    static shared_ptr<CefModule> instance_;

  private:
    RenderHandler *renderHandler;
    CefRefPtr<CefBrowser> browser;
    CefRefPtr<BrowserClient> browserClient;
};

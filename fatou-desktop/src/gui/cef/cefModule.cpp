#include "cefModule.h"
#include "cefApp.h"
#include "../../window/sharedTexture.h"
#include "../../window/console.h"

#include "js.h"

std::shared_ptr<CefModule> CefModule::instance_;

void CefModule::startup(HINSTANCE mod) {
    assert(!instance_.get());
    instance_ = make_shared<CefModule>(mod);
    instance_->thread_ = make_shared<std::thread>(
        std::bind(&CefModule::message_loop, instance_.get()));

    { // wait for message loop to initialize

        std::unique_lock<std::mutex> lock(instance_->lock_);
        std::weak_ptr<CefModule> weak_self(instance_);
        instance_->signal_.wait(lock, [weak_self]() {
            auto const mod = weak_self.lock();
            if (mod) {
                return mod->ready_.load();
            }
            return true;
        });
    }

    // log_message("cef module is ready\n");
}

void CefModule::shutdown() {
    assert(instance_.get());
    if (instance_) {
        if (instance_->thread_) {
            CefRefPtr<CefTask> task(new QuitTask());
            CefPostTask(TID_UI, task.get());
            instance_->thread_->join();
            instance_->thread_.reset();
        }
        instance_.reset();
    }
}

void CefModule::loop_init() {
    // log_message("cef initializing ... \n");

    CefSettings settings;

    // checkout detailed settings options
    // http://magpcss.org/ceforum/apidocs/projects/%28default%29/_cef_settings_t.html
    // nearly all the settings can be set via args too.
    // CefString(&settings.browser_subprocess_path).FromASCII("sub_proccess
    // path, by default uses and starts this executeable as child");
    // CefString(&settings.cache_path).FromASCII("");
    // CefString(&settings.log_file).FromASCII("");
    // settings.log_severity = LOGSEVERITY_DEFAULT;
    // CefString(&settings.resources_dir_path).FromASCII("");
    // CefString(&settings.locales_dir_path).FromASCII("");
    settings.no_sandbox = true;

    // TODO: Can this improve the delays when rendering heavy stuff and
    // interacting with the UI? This settings is recommended anyway
    settings.multi_threaded_message_loop = false; // true for windows only
    settings.windowless_rendering_enabled = true;

    CefRefPtr<MyCefApp> app(new MyCefApp());

    CefMainArgs main_args(module_);

    if (!CefInitialize(main_args, settings, app, nullptr)) {
        throw runtime_error("CEF init failed!");
    }

    // log_message("cef is initialized.\n");

    // signal cef is initialized and ready
    ready_ = true;
    signal_.notify_one();
}

void CefModule::loop_setupRenderer() {

    // TODO: load from main
    renderHandler = new RenderHandler(800, 800);

    // create browser-window

    CefWindowInfo window_info;
    CefBrowserSettings browserSettings;

    browserSettings.background_color = 0;
    browserSettings.windowless_frame_rate = 60; // 30 is default

    // in linux set a gtk widget, in windows a hwnd. If not available
    // set nullptr - may cause some render errors, in context-menu and
    // plugins.
    std::unique_lock lk(hwndReadyMutex);
    HWND windowHandle = shared_hwnd;
    // renderSystem->getAutoCreatedWindow()->getCustomAttribute("WINDOW",
    //                                                           &windowHandle);
    window_info.SetAsWindowless(windowHandle);

    // we want to use OnAcceleratedPaint
    // window_info.shared_texture_enabled = true;

    // TODO
    // we are going to issue calls to SendExternalBeginFrame
    // and CEF will not use its internal BeginFrameTimer in this case
    // window_info.external_begin_frame_enabled = true;

    browserClient = new BrowserClient(renderHandler);

    browser = CefBrowserHost::CreateBrowserSync(
        window_info, browserClient.get(),
        // "https://dvcs.w3.org/hg/d4e/raw-file/tip/mouse-event-test.html",
        //"wikipedia.org",
        //"https://fuzzbomb.github.io/accessibility-demos/html5-autofocus-demo.html",
        //  "https://example.com/test",
        "https://app/index.html",
        //"https://loading.io/spinner/",
        //
        browserSettings, nullptr, nullptr);

    mainBrowser = browser;

    // browserClient->sendMessage(browser);

    // inject user-input by calling - non-trivial for non-windows -
    // checkout the cefclient source and the platform specific cpp, like
    // cefclient_osr_widget_gtk.cpp for linux
    // browser->GetHost()->SendKeyEvent(...);
    // browser->GetHost()->SendMouseMoveEvent(...);
    // browser->GetHost()->SendMouseClickEvent(...);
    // browser->GetHost()->SendMouseWheelEvent(...);
}

void CefModule::loop_loop() {

    std::chrono::duration frameTime =
        std::chrono::milliseconds(1000 / 60); // 60Hz
    auto nextFrame = std::chrono::system_clock::now();
    auto lastFrame = nextFrame - frameTime;

    while (mainLoopRunning) {
        if (browser->GetHost()->GetZoomLevel() != targetZoomLevel) {
            browser->GetHost()->SetZoomLevel(targetZoomLevel);
        }

        while (!inputEventQueue.empty()) {
            const InputEventData data = inputEventQueue.dequeue();
            if (data.type == MY_KEY_EVENT)
                browser->GetHost()->SendKeyEvent(data.keyEvent);
            else if (data.type == MY_CHAR_EVENT)
                browser->GetHost()->SendKeyEvent(data.keyEvent);
            else if (data.type == MY_MOUSE_MOVE_EVENT)
                browser->GetHost()->SendMouseMoveEvent(data.mouseEvent,
                                                       data.mouseLeave);
            else if (data.type == MY_MOUSE_WHEEL_EVENT)
                browser->GetHost()->SendMouseWheelEvent(data.mouseEvent,
                                                        data.dx, data.dy);
            else if (data.type == MY_MOUSE_CLICK_EVENT)
                browser->GetHost()->SendMouseClickEvent(
                    data.mouseEvent, data.mouseButton, data.mouseUp,
                    data.clickCount);
        }

        // TODO: when vulkan is busy, do message loop work is extremely slow
        // (like, >10s, even if vulkan is at 30FPS)
        // disabling gpu slightly improves the problem, but doesn't really solve
        // it
        CefDoMessageLoopWork();

        std::this_thread::sleep_until(nextFrame);

        lastFrame = nextFrame;
        nextFrame += frameTime;
    }

    /*
    // main_message_loop_external_pump_win.cc

      // We need to run the message pump until it is idle. However we don't
    have
      // that information here so we run the message loop "for a while".
      for (int i = 0; i < 10; ++i) {
        // Do some work.
        CefDoMessageLoopWork();

        // Sleep to allow the CEF proc to do work.
        Sleep(50);
      }
      */

    ////////////////////////////////////////////////////

    // CefRunMessageLoop();
}

void CefModule::loop_clean() {

    ////////////////////////////////////////////////////

    //  log_message("cef shutting down ... \n");

    // Important: make sure to destroy all references to the Browser before
    // calling Shutdown
    mainBrowser.reset();
    browser.reset();
    browserClient.reset();
    CefShutdown();
    //  log_message("cef is shutdown\n");
}

void CefModule::message_loop() {

    try {
        loop_init();
        loop_setupRenderer();
        loop_loop();
        loop_clean();
    } catch (const std::exception &error) {
        fatalBox("std::exception: " + string(error.what()));
        return;
    } catch (...) {
        fatalBox("unknown exception");
        return;
    }
}

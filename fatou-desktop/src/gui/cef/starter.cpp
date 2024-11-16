#include "starter.h"

#include "cefModule.h"
#include "cefApp.h"
//#include "webView.h"

#include "cefUtil.h"

#include <iostream>



Starter::Starter(HINSTANCE instance) : mainArgs(instance) {
    std::cout << "Using CEF v" << cef_version() << std::endl;

    // Enable High-DPI support on Windows 7 or newer.
    CefEnableHighDPISupport();

    // Create a temporary CommandLine object.
    CefRefPtr<CefCommandLine> command_line = CreateCommandLine(mainArgs);

    isBrowserProcess = false;
    switch (GetProcessType(command_line)) {
    case PROCESS_TYPE_BROWSER:
        isBrowserProcess = true;
        std::cout << "browser process type" << std::endl;
        break;
    case PROCESS_TYPE_RENDERER:
        std::cout << "renderer process type" << std::endl;
        break;
    case PROCESS_TYPE_OTHER:
    default:
        std::cout << "other process type" << std::endl;
    }

    if (isMainProcess()) {
        CefModule::startup(instance);
    }
}

int Starter::runChildProcess() {
    // check first if we need to run as a worker process
    if (!isMainProcess()) {
        CefRefPtr<MyCefApp> app(new MyCefApp());
        int exit_code = CefExecuteProcess(mainArgs, app, nullptr);
        if (exit_code >= 0) {
            // running as child process
            return exit_code;
        }
    }

    return -1;
}

Starter::~Starter() {
    if (isMainProcess()) {
        CefModule::shutdown();
    }
}
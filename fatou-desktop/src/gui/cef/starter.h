#pragma once

#define NOMINMAX
#include "windows.h"

#include <include/cef_app.h>
#include <include/cef_browser.h>

// Lifetime management for CEF components.
// Manages the CEF message loop and CefInitialize/CefShutdown
class Starter {
  public:
    Starter(HINSTANCE);
    ~Starter();

    bool isMainProcess() const { return isBrowserProcess; }
    int runChildProcess();


  private:
    CefMainArgs mainArgs;
    bool isBrowserProcess;
};
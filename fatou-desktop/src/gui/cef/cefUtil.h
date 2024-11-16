#pragma once

#include <include/cef_app.h>

// Process types that may have different CefApp instances.
enum ProcessType {
    PROCESS_TYPE_BROWSER,
    PROCESS_TYPE_RENDERER,
    PROCESS_TYPE_OTHER,
};

string cef_version();

CefRefPtr<CefCommandLine> CreateCommandLine(const CefMainArgs &main_args);

ProcessType GetProcessType(const CefRefPtr<CefCommandLine> &command_line);
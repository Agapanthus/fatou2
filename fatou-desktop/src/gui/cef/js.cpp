
#include "js.h"
#include <queue>
#include <boost/regex.hpp>
#include <iostream>
#include "../../window/sharedTexture.h"

#ifdef WITH_GUI
CefRefPtr<CefBrowser> mainBrowser = nullptr;
#endif

std::queue<string> deferred;

void sendJS(const string &js) {

#ifdef WITH_GUI
    if (!mainBrowser || !jsConnected) {
        // safe for later
        deferred.push(js);
        return;
    }
    CefRefPtr<CefFrame> frame = mainBrowser->GetMainFrame();

    while (!deferred.empty()) {
        frame->ExecuteJavaScript(deferred.front(), frame->GetURL(), 0);
        deferred.pop();
    }

    frame->ExecuteJavaScript(js, frame->GetURL(), 0);
#endif
}

string jsStr(const string &inp) {
    return "\"" + boost::regex_replace(inp, boost::regex("\""), "\\\"") + "\"";
}
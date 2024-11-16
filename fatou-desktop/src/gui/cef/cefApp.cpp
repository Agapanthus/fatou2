#include "cefApp.h"

void MyCefApp::OnBeforeCommandLineProcessing(
    CefString const & /*process_type*/,
    CefRefPtr<CefCommandLine> command_line) {
    // see https://peter.sh/experiments/chromium-command-line-switches/

    // TODO: 

    // disable creation of a GPUCache/ folder on disk
    command_line->AppendSwitch("disable-gpu-shader-disk-cache");

    // does this do anything?
    command_line->AppendSwitch("off-screen-rendering-enabled");

    command_line->AppendSwitchWithValue("disable-gpu", "1");
    command_line->AppendSwitchWithValue("disable-gpu-vsync", "1");
    command_line->AppendSwitchWithValue("disable-gpu-compositing", "1");
    command_line->AppendSwitchWithValue("disable-accelerated-2d-canvas", "1");
    command_line->AppendSwitchWithValue("disable-gpu-rasterization", "1");
    command_line->AppendSwitchWithValue("disable-gpu-sandbox", "1");
    command_line->AppendSwitchWithValue("disable-gpu-shader-disk-cache", "1");

    // command_line->AppendSwitch("disable-accelerated-video-decode");

    // un-comment to show the built-in Chromium fps meter
    // command_line->AppendSwitch("show-fps-counter");

    // command_line->AppendSwitch("disable-gpu-vsync");

    // Most systems would not need to use this switch - but on older
    // hardware, Chromium may still choose to disable D3D11 for gpu
    // workarounds. Accelerated OSR will not at all with D3D11 disabled, so
    // we force it on.
    //
    // See the discussion on this issue:
    // https://github.com/daktronics/cef-mixer/issues/10
    //
    // command_line->AppendSwitchWithValue("use-angle", "d3d11");

    // tell Chromium to autoplay <video> elements without
    // requiring the muted attribute or user interaction
    // command_line->AppendSwitchWithValue("autoplay-policy",
    //                                     "no-user-gesture-required");
}

bool MyCefApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
                                        CefProcessId /*source_process*/,
                                        CefRefPtr<CefProcessMessage> message) {
    auto const name = message->GetName().ToString();
    if (name == "mixer-update-stats") {
        if (mixer_handler_ != nullptr) {
            // we expect just 1 param that is a dictionary of stat values
            auto const args = message->GetArgumentList();
            auto const size = args->GetSize();
            if (size > 0) {
                auto const dict = args->GetDictionary(0);
                if (dict && dict->GetSize() > 0) {
                    mixer_handler_->update(dict);
                }
            }
        }
        return true;
    }
    return false;
}
#include "nativeWindow.h"
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
// TODO: Is this fine?
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>

#include <shellscalingapi.h> // GetDpiForMonitor

#include "../gui/cef/js.h"

void getMonitorDPI() {
    // https://docs.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows?redirectedfrom=MSDN
    ::EnumDisplayMonitors(
        nullptr, nullptr,
        [](HMONITOR Arg1, HDC Arg2, LPRECT Arg3, LPARAM Arg4) {
            MONITORINFOEXA mif;
            mif.cbSize = sizeof(MONITORINFOEXA);
            std::map<string, string> info;

            if (::GetMonitorInfoA(Arg1, &mif) != 0) {
                /*std::cout << mif.szDevice << '\n';
                std::cout << "monitor rect:    " << '(' << mif.rcMonitor.left
                          << ',' << mif.rcMonitor.top << ")-" << '('
                          << mif.rcMonitor.right << ',' << mif.rcMonitor.bottom
                          << ")\n";
                std::cout << "work rect:       " << '(' << mif.rcWork.left
                          << ',' << mif.rcWork.top << ")-" << '('
                          << mif.rcWork.right << ',' << mif.rcWork.bottom
                          << ")\n";*/

                info["handle"] = jsStr((int)Arg1);
                info["name"] = jsStr(mif.szDevice);
                info["rect"] = "[" + to_string(mif.rcMonitor.top) + "," +
                               to_string(mif.rcMonitor.right) + "," +
                               to_string(mif.rcMonitor.bottom) + "," +
                               to_string(mif.rcMonitor.left) + "]";
                info["work"] = "[" + to_string(mif.rcWork.top) + "," +
                               to_string(mif.rcWork.right) + "," +
                               to_string(mif.rcWork.bottom) + "," +
                               to_string(mif.rcWork.left) + "]";
            }

            UINT xdpi, ydpi;
            LRESULT success =
                ::GetDpiForMonitor(Arg1, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
            if (success == S_OK) {
                // std::cout << "DPI (effective): " << xdpi << ',' << ydpi <<
                // '\n';
                info["effective dpi"] =
                    "[" + to_string(xdpi) + "," + to_string(ydpi) + "]";
            }
            success = ::GetDpiForMonitor(Arg1, MDT_ANGULAR_DPI, &xdpi, &ydpi);
            if (success == S_OK) {
                // std::cout << "DPI (angular):   " << xdpi << ',' << ydpi <<
                // '\n';
                info["angular dpi"] =
                    "[" + to_string(xdpi) + "," + to_string(ydpi) + "]";
            }
            success = ::GetDpiForMonitor(Arg1, MDT_RAW_DPI, &xdpi, &ydpi);
            if (success == S_OK) {
                // std::cout << "DPI (raw):       " << xdpi << ',' << ydpi <<
                // '\n';
                info["raw dpi"] =
                    "[" + to_string(xdpi) + "," + to_string(ydpi) + "]";
            }
            DEVMODEA dm;
            dm.dmSize = sizeof(DEVMODEA);
            if (::EnumDisplaySettingsA(mif.szDevice, ENUM_CURRENT_SETTINGS,
                                       &dm) != 0) {
                /* std::cout << "BPP:             " << dm.dmBitsPerPel << '\n';
                 std::cout << "resolution:      " << dm.dmPelsWidth << ','
                           << dm.dmPelsHeight << '\n';
                 std::cout << "frequency:       " << dm.dmDisplayFrequency
                           << '\n';*/

                info["frequency"] = jsStr(dm.dmDisplayFrequency);
                info["resolution"] = "[" + to_string(dm.dmPelsWidth) + "," +
                                     to_string(dm.dmPelsHeight) + "]";
                info["BPP"] = jsStr(dm.dmBitsPerPel);
            }
            // std::cout << '\n';
            commitJS("appendMonitor", info);
            return TRUE;
        },
        0);
}

double thisMonitorZoom(HWND hWnd) {
    double zoom = GetDpiForWindow(hWnd) / 96.0;

    HMONITOR hm = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
    setJS("current monitor", size_t(hm));
    sendJS("window.app.store.commit(\"setCurrentMonitor\"," + to_string(size_t(size_t(hm))) + ")");

    return zoom;
}

HWND getNativeFromGLFW(GLFWwindow *window) {
    return glfwGetWin32Window(window);
}

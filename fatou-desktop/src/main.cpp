#include "window/app.h"
#include "window/console.h"

#include <iostream>

#ifdef WITH_GUI
#include "gui/cef/starter.h"
#endif

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PWSTR pCmdLine, int nCmdShow) {
    DebugConsole console;
    try {
#ifdef WITH_GUI
        Starter starter(hInstance);
        if (!starter.isMainProcess()) {
            return starter.runChildProcess();
        }
#endif

        console.start(1024);

        App app;
        app.run();

    } catch (const std::runtime_error &error) {
        fatalBox("runtime error: " + string(error.what()));
        return EXIT_FAILURE;
    } catch (const std::exception &error) {
        fatalBox("std::exception: " + string(error.what()));
        return EXIT_FAILURE;
    } catch (...) {
        fatalBox("unknown exception");
        return EXIT_SUCCESS;
    }
    console.wait();
    return EXIT_SUCCESS;
}
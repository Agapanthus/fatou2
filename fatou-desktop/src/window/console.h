#pragma once

#include <cstdint>
#include <stdexcept>
#include <iostream>

bool ReleaseConsole();
bool RedirectConsoleIO();
void AdjustConsoleBuffer(int16_t minLength);
bool CreateNewConsole(int16_t minLength);

#ifdef _DEBUG
#define HAS_CONSOLE
#endif

// show always
//#define HAS_CONSOLE


void fatalBox(const string &msg);
class DebugConsole : private boost::noncopyable {
  public:
    DebugConsole() { started = false; }

    void start(int minLength) {
#ifdef HAS_CONSOLE
        if (!CreateNewConsole(minLength)) {
            throw std::runtime_error("couldn't create console");
        }
        started = true;
#endif
    }

    void wait() {
        if (started)
            system("pause");
    }

    ~DebugConsole() {
        if (started)
            ReleaseConsole();
    }

  private:
    bool started;
};
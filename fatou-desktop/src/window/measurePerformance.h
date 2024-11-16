#pragma once
#include <iostream>


class MeasurePerformance {

  public:
    MeasurePerformance(const string &what) : what(what) {
        startTime = std::chrono::high_resolution_clock::now();
    }

    ~MeasurePerformance() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        // in seconds
        float time = std::chrono::duration<float, std::chrono::seconds::period>(
                         currentTime - startTime)
                         .count();

        std::cout << what << " took " << time*1000 << "ms" << std::endl;
    }

    const string what;
    std::chrono::time_point<std::chrono::steady_clock> startTime;
};
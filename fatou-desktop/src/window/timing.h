#pragma once

#include "logicalDevice.h"
#include <numeric>
#include <boost/math/special_functions/lambert_w.hpp>

inline static double sqr(double x) { return x * x; }

// https://stackoverflow.com/a/19040841/6144727
inline int linreg(int n, const double x[], const double y[], double &m,
                  double &b /*, double &r*/) {
    double sumx = 0.0;  /* sum of x     */
    double sumx2 = 0.0; /* sum of x**2  */
    double sumxy = 0.0; /* sum of x * y */
    double sumy = 0.0;  /* sum of y     */
    double sumy2 = 0.0; /* sum of y**2  */

    for (int i = 0; i < n; i++) {
        sumx += x[i];
        sumx2 += sqr(x[i]);
        sumxy += x[i] * y[i];
        sumy += y[i];
        sumy2 += sqr(y[i]);
    }

    double denom = (n * sumx2 - sqr(sumx));
    if (denom == 0) {
        // singular matrix. can't solve the problem.
        m = 0;
        b = 0;
        // r = 0;
        return 1;
    }

    m = (n * sumxy - sumx * sumy) / denom;
    b = (sumy * sumx2 - sumx * sumxy) / denom;
    // r = (sumxy - sumx * sumy / n) / /* compute correlation coeff */
    //     sqrt((sumx2 - sqr(sumx) / n) * (sumy2 - sqr(sumy) / n));

    return 0;
}

inline string formatBig(uint64_t n) {
    if (n > 2000 * 1000 * 1000) {
        return to_string(n / 1000 / 1000 / 1000) + "G";
    }
    if (n > 2000 * 1000) {
        return to_string(n / 1000 / 1000) + "M";
    }
    if (n > 2000) {
        return to_string(n / 1000) + "K";
    }
    return to_string(n);
}

class EffortEstimator {
  public:
    EffortEstimator(size_t cacheSize = 100) : cacheSize(cacheSize) {
        cacheTime.resize(cacheSize);
        cacheEffort.resize(cacheSize);

        cacheEPS.resize(cacheSize);
        cacheLog.resize(cacheSize);
    }

    void reset() {
        curPos = -1;
        filled = 0;
    }

    double predictTime(size_t samples) {
        double m;
        double b;

        // better model then using linear stuff
        linreg(filled, cacheLog.data(), cacheEPS.data(), m, b);
        // std::cout << m << " " << b << " " << r << std::endl;

        return samples / (b + m * std::log(samples));
    }

    uint64_t predictSamples(double time) {
        double a;
        double b;

        // better model then using linear stuff (I guess, or not?)
        linreg(filled, cacheLog.data(), cacheEPS.data(), b, a);

        // x = s / (a + b * log(s)) solve for s
        // -b x ProductLog(-e^(-a/b)/(b x))
        if (b != 0)
            try {
                return -b * time *
                       boost::math::lambert_wm1(-std::exp(-a / b) / (b * time));
            } catch (std::exception) {
            }

        return 0;
    }

    uint64_t predictSamplesLinear(double time) {
        double a;
        double b;
        linreg(filled, cacheTime.data(), cacheEffort.data(), b, a);
        uint64_t samples = a + b * time;

        uint64_t samplesPessi = 1000;
        const double tol = 2.0;
        if (curPos >= 0)
            samplesPessi = cacheEffort[curPos] / cacheTime[curPos] * time * tol;

        return std::min(samplesPessi, samples);
    }

    void push(double time, uint64_t samples, double averageDifficulty) {
        curPos = (curPos + 1) % cacheSize;
        cacheTime[curPos] = time;
        double effort = samples * averageDifficulty;
        cacheEffort[curPos] = effort;

        cacheLog[curPos] = std::log(effort);
        cacheEPS[curPos] = effort / time;

        filled = std::min(filled + 1, cacheSize);

        // std::cout << samples <<", " << time << "," << std::endl;
    }

  private:
    vector<double> cacheTime;
    vector<double> cacheEffort;
    vector<double> cacheLog;
    vector<double> cacheEPS;
    int curPos = -1;
    const size_t cacheSize;
    size_t filled = 0;
};

class TimeQueryPool {
  public:
    TimeQueryPool(shared_ptr<LogicalDevice> device, const size_t phases)
        : device(device), phases(phases) {
        vk::QueryPoolCreateInfo createInfo{};
        createInfo.flags = {};
        createInfo.pipelineStatistics =
            {}; // vk::QueryPipelineStatisticFlagBits::eFragmentShaderInvocations;
        createInfo.queryCount = phases * 2;
        createInfo.queryType = vk::QueryType::eTimestamp;
        timeQueryPool = device->device.createQueryPool(createInfo);

        results.resize(phases);
        userdata.resize(phases);
        loaded.resize(phases);
        for (size_t i = 0; i < phases; i++) {
            loaded[i] = false;
        }
    }

    // must be called before beginRenderPass
    void start(vk::CommandBuffer commandBuffer, const uint32_t id,
               const uint64_t userdata) {
        assert(id < phases);

        // reset cache
        results[id].reset();
        this->userdata[id] = userdata;
        this->loaded[id] = true;

        // There's an implicit execution dependency for these query-related
        // commands, so the reset will finish before the timestamps are written.
        commandBuffer.resetQueryPool(*timeQueryPool, id * 2, 2);
        commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eTopOfPipe,
                                     *timeQueryPool, id * 2);
    }

    // must be called directly after endRenderPass
    void stop(vk::CommandBuffer commandBuffer, const uint32_t id) {
        assert(id < phases);

        commandBuffer.writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe,
                                     *timeQueryPool, id * 2 + 1);
    }

    void fetch(const uint32_t id) {

        // timer is not loaded!
        if (!loaded[id])
            return;

        // device->physical->device
        const auto [result, buffer] = timeQueryPool.getResults<uint64_t>(
            id * 2, 2, sizeof(uint64_t) * 2, sizeof(uint64_t),
            vk::QueryResultFlagBits::e64);

        if (result == vk::Result::eNotReady) {
            return;
        } else if (result == vk::Result::eSuccess) {
            assert(buffer[1] >= buffer[0]);
            // ticks -> ns -> s
            results[id] =
                double(buffer[1] - buffer[0]) *
                (double(device->physical->properties.limits.timestampPeriod) /
                 (1000. * 1000. * 1000.));
        } else {
            throw runtime_error("Failed to receive query results!");
        }

        // value not loaded anymore
        loaded[id] = false;
    }

    optional<pair<double, uint64_t>> getTime(const uint32_t id) {
        if (!results[id].has_value() || !userdata[id].has_value())
            return {};

        auto res = pair(results[id].value(), userdata[id].value());
        userdata[id].reset();
        return res;
    }

  private:
    shared_ptr<LogicalDevice> device;
    vk::raii::QueryPool timeQueryPool = nullptr;
    vector<optional<double>> results;
    vector<optional<uint64_t>> userdata;
    vector<bool> loaded;
    const size_t phases;
};

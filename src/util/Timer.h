#pragma once

#include <chrono>

// Small utility for measuring elapsed time.
class Timer {
public:
    void start() { start_ = clock::now(); running_ = true; }
    void stop() { end_ = clock::now(); running_ = false; }

    // Elapsed milliseconds between start and stop (or now if running).
    long long elapsedMs() const {
        auto e = running_ ? clock::now() : end_;
        return std::chrono::duration_cast<std::chrono::milliseconds>(e - start_).count();
    }

private:
    using clock = std::chrono::steady_clock;
    clock::time_point start_{};
    clock::time_point end_{};
    bool running_{false};
};

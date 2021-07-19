#include <chrono>

class Timer {
    int& o_cnt;
    double& o_time;
    std::chrono::time_point<std::chrono::steady_clock> begin;

public:
    Timer(int& o_cnt, double& o_time) : o_cnt(o_cnt), o_time(o_time), begin(std::chrono::steady_clock::now()) {}

    ~Timer() {
        ++o_cnt;
        auto end = std::chrono::steady_clock::now();
        o_time += static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count()) / 1e6;
    }
};

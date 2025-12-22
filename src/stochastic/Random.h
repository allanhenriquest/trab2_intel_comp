#pragma once

#include <random>

using namespace std;

// Simple RNG wrapper for reproducibility.
class Random {
public:
    explicit Random(unsigned long long seed = 42) : rng_(seed) {}

    // Uniform real in [a, b)
    double uniform(double a = 0.0, double b = 1.0) { return uniform_real_distribution<double>(a, b)(rng_); }
    // Uniform integer in [a, b]
    int randint(int a, int b) { return uniform_int_distribution<int>(a, b)(rng_); }
    // Bernoulli with probability p
    bool bernoulli(double p) { return bernoulli_distribution(p)(rng_); }

private:
    mt19937_64 rng_;
};

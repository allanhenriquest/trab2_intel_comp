#pragma once

#include <random>
#include <cmath>
#include <limits>

using namespace std;

// Simple RNG wrapper for reproducibility.
class Random {
public:
    explicit Random(unsigned long long seed = 42) : rng_(seed) {}

    // Uniform real in [a, b)
    double real(double a = 0.0, double b = 1.0) { return uniform_real_distribution<double>(a, b)(rng_); }
    
    // Uniform integer in [a, b]
    int integer(int a, int b) { return uniform_int_distribution<int>(a, b)(rng_); }
    
    // Bernoulli with probability p
    bool bernoulli(double p) { return bernoulli_distribution(p)(rng_); }

    // Log-normal sample parameterized by expected value and variance
    double lognormal(double expected, double variance) {
        if (expected <= 0.0 || variance < 0.0) return numeric_limits<double>::quiet_NaN();
        double s2 = log(1.0 + variance / (expected * expected));
        double mu = log(expected) - 0.5 * s2;
        return lognormal_distribution<double>(mu, abs(sqrt(s2)))(rng_);
    }

private:
    mt19937_64 rng_;
};

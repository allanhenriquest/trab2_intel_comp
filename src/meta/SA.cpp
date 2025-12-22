#include "SA.h"

using namespace std;

SA::SA(SAParams params) : params_(params) {}

Solution SA::refine(const Instance& inst, const Solution& seed, const CostEstimator& estimator) {
    (void)inst; (void)seed; (void)estimator;
    // Placeholder: return seed for now (copy elided by NRVO).
    return seed;
}

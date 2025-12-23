#include "MonteCarlo.h"
#include "Random.h"
#include <limits>

using namespace std;

double MonteCarlo::expectedCost(const Instance& inst, const Solution& sol, 
    const Solution& best_deter_sol, int samples, int k) {
    if (samples <= 0) return sol.total_cost;

    const int n = inst.n;
    // Guard sizes
    if (n <= 0 || (int)sol.openFacilities.size() < n) return sol.total_cost;

    // Fixed opening costs (deterministic)
    double fixed_opening = 0.0;
    for (int i = 0; i < n; ++i) {
        if (sol.openFacilities[i]) fixed_opening += inst.opening_cost;
    }

    double Cmax = 0.0;
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < n; ++i) {
            if (best_deter_sol.openFacilities[i])
                Cmax = max(Cmax, inst.allocation_costs[i][j]);
        }
    }

    const double EPS = 1e-12; // avoid zero means in lognormal

    Random rng(123456789ULL);
    double total = 0.0;
    for (int s = 0; s < samples; ++s) {
        double scenario = fixed_opening;
        for (int j = 0; j < n; ++j) {
            double best = numeric_limits<double>::infinity();
            for (int i = 0; i < n; ++i) {
                if (!sol.openFacilities[i]) continue;
                double mean = inst.allocation_costs[i][j];
                if (mean < 0.0) continue; // skip invalid costs
                double sample = rng.lognormal(max(mean, EPS), k);
                if (sample < best) best = sample;
            }
            if (best < numeric_limits<double>::infinity()) scenario += best;
            if (best > Cmax) scenario += 2 * inst.opening_cost; // penalty for extreme costs
        }
        total += scenario;
    }

    return total / samples;
}

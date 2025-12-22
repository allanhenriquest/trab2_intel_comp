#include "MonteCarlo.h"

using namespace std;

double MonteCarlo::expectedCost(const Instance& inst, const Solution& sol, int samples) const {
    (void)inst; (void)sol; (void)samples;
    // Placeholder: deterministic cost passthrough could be used here later.
    return sol.total_cost;
}

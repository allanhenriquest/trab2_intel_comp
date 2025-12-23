#include "Solver.h"

Solution Solver::solveInstance(const Instance& instance, const GAParams& ga_params, 
    const SAParams& sa_params, int mc_samples, int mc_k) 
{
    return NULL;
}

vector<Solution> Solver::solveDeterministic(const Instance& instance, const GAParams& ga_params) {
    GA ga(ga_params);
    return ga.run(instance, ga_params.elite_k);
}

vector<Solution> Solver::solveStochastic(const Instance& instance, const SAParams& sa_params, 
    const vector<Solution>& initialPool, const Solution& best_deter_sol, int mc_samples, int mc_k) 
{
    return {};
}
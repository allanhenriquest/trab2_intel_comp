#pragma once

#include "model/Solution.h"
#include "io/Instance.h"
#include "meta/GA.h"
#include "meta/SA.h"
#include "model/Evaluator.h"
#include "model/MonteCarlo.h"
#include <vector>

using namespace std;

class Solver
{
public:

    static Solution solveInstance(const Instance& instance, const GAParams& ga_params,
        const SAParams& sa_params, int mc_samples, int mc_k);
    
private:
    
    static vector<Solution> solveDeterministic(const Instance& instance, const GAParams& ga_params);
    static vector<Solution> solveStochastic(const Instance& instance, const SAParams& sa_params,
        const vector<Solution>& initialPool, const Solution& best_deter_sol, int mc_samples, int mc_k);
        
};
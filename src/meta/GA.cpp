#include "GA.h"
#include "model/Evaluator.h"

using namespace std;

GA::GA(GAParams params, Instance instance) : params_(params), instance_(instance) {}

vector<Solution> GA::run(const Instance& inst, int elite_k_override) {
    (void)inst; (void)elite_k_override;
    // Placeholder: return empty list for now.
    return {};
}

void GA::initializePopulation() 
{
    for(int i=0; i < params_.pop_size; ++i) {
        Solution sol(instance_.n);
        for(int j=0; j < instance_.n; ++j) {
            sol.openFacilities[j] = (rand() % 2 == 0);
        }
        sol.ensureAtLeastOneOpen();
        Evaluator::evaluateFull(instance_, sol);
        population.push_back(sol);
    }
}

void GA::nextGeneration() { (void)instance_; }

void GA::evaluatePopulation()
{ 

}


vector<Solution> GA::selectElite(int k) const { (void)k; return {}; }

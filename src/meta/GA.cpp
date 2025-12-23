#include "GA.h"

using namespace std;

GA::GA(GAParams params) : params_(params) {}

vector<Solution> GA::run(const Instance& inst, int elite_k_override) {
    (void)inst; (void)elite_k_override;
    // Placeholder: return empty list for now.
    return {};
}

void GA::initializePopulation(const Instance& inst) { (void)inst; }
void GA::evaluatePopulation(const Instance& inst) { (void)inst; }
void GA::nextGeneration(const Instance& inst) { (void)inst; }
vector<Solution> GA::selectElite(int k) const { (void)k; return {}; }

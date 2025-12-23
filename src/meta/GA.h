#pragma once

#include <vector>
#include <functional>
#include "io/Instance.h"
#include "model/Solution.h"

using namespace std;

struct GAParams {
    int pop_size{50};
    int max_generations{200};
    double crossover_rate{0.8};
    double mutation_rate{0.02};
    int elite_k{5};
    unsigned long long seed{42};
};

// Genetic Algorithm for deterministic phase (chromosome = y vector).
class GA {
public:
    explicit GA(GAParams params);

    // Run GA and return top-k elite solutions evaluated deterministically.
    std::vector<Solution> run(const Instance& inst, int elite_k_override = -1);

private:
    GAParams params_;

    // Internal helpers (signatures only; implementations are placeholders).
    void initializePopulation(const Instance& inst);
    void evaluatePopulation(const Instance& inst);
    void nextGeneration(const Instance& inst);
    std::vector<Solution> selectElite(int k) const;
};

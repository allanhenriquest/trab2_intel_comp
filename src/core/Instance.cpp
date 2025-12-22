#include "Instance.h"
#include <stdexcept>

using namespace std;

Instance::Instance(int n_) : n(n_), opening_costs(n_, 0.0), allocation_costs(n_, std::vector<double>(n_, 0.0)) {}

void Instance::validate() const {
    if (n < 1) throw runtime_error("Instance: n must be positive");
    if ((int)opening_costs.size() != n) throw runtime_error("Instance: opening_costs size mismatch");
    if ((int)allocation_costs.size() != n) throw runtime_error("Instance: allocation_costs rows mismatch");
    for (const auto& row : allocation_costs) {
        if ((int)row.size() != n) throw runtime_error("Instance: allocation_costs columns mismatch");
    }
}

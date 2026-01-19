#pragma once
#include "io/Instance.h"
#include "util/Metrics.h"
#include <string>

class Writer {
public:
    // Salva métricas do GA em CSV.
    // 'subfolder' permite organizar resultados por configuração.
    static void saveGaStats(const Instance& inst, const GaRunMetrics& metrics, std::string subfolder = "");
};
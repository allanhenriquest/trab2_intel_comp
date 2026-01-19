#pragma once
#include <string>
#include <vector>
#include "model/Solution.h"
#include "util/Metrics.h"

class Writer {
public:
    // Garante que o diretório exista (cria se não existir)
    static void ensureDirectory(const std::string& path);

    // Limpa um diretório específico ou arquivo
    static void cleanUpDirectory(const std::string& path);
    static void cleanUpFile(const std::string& path);
    
    // Escreve uma solução completa em arquivo (Formato legível)
    static void writeSolution(const std::string& filename, const Solution& sol, unsigned long long seed);

    // Salva os parâmetros usados na execução
    static void saveParameters(const GAParams& ga_params, const SAParams& sa_params, const std::string& filename);
    
    // Adiciona uma linha a um CSV (Cria o arquivo com cabeçalho se não existir)
    static void appendCSV(const std::string& filename, const std::string& header, const std::string& content);
    
    // Chama o script Python para gerar gráficos automaticamente
    static void createChart(const std::string& instance_name);
    
    // Mantido para compatibilidade (se ainda for usado em algum lugar)
    static void saveGaStats(const std::string& instance_name, const GaRunMetrics& metrics, const std::string& subfolder = "");

    static void saveSaStats(const std::string& instance_name, int run_id, const std::vector<SaStep>& history);
};
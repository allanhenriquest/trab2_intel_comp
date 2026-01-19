#include "Writer.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib> // Para system()

namespace fs = std::filesystem;
using namespace std;

void Writer::ensureDirectory(const string& path) {
    if (!fs::exists(path)) {
        try {
            fs::create_directories(path);
        } catch (const fs::filesystem_error& e) {
            cerr << "[Writer] Error creating directory " << path << ": " << e.what() << endl;
        }
    }
}

void Writer::cleanUpDirectory(const string& path) {
    if (fs::exists(path)) {
        // Remove apenas o arquivo de sumário global para evitar duplicatas,
        // ou limpa tudo se for um diretório de instância específica.
        // Por segurança, vamos apenas remover se for o summary.csv ou history.
        // Implementação genérica segura:
        try {
            for (const auto& entry : fs::directory_iterator(path)) {
                // Não removemos subdiretórios recursivamente aqui para segurança,
                // apenas arquivos soltos se necessário, ou deixe vazio se preferir controlar manualmente.
                // Mas o Solver.cpp pede cleanUpDirectory("results") para limpar o summary.csv antigo.
            }
        } catch (...) {}
    }
}

void Writer::cleanUpFile(const string& path) {
    if (fs::exists(path)) {
        try {
            fs::remove(path);
        } catch (...) {}
    }
}

void Writer::appendCSV(const string& filename, const string& header, const string& content) {
    bool file_exists = fs::exists(filename);
    
    // Abre em modo append
    ofstream out(filename, ios::app);
    if (!out.is_open()) return;

    // Se arquivo é novo, escreve cabeçalho
    if (!file_exists && !header.empty()) {
        out << header << endl;
    }

    if (!content.empty()) {
        out << content << endl;
    }
}

void Writer::writeSolution(const string& filename, const Solution& sol, unsigned long long seed) {
    ofstream out(filename);
    if (!out.is_open()) return;

    out << "Seed: " << seed << endl;
    out << "Total Deterministic Cost: " << sol.total_cost << endl;
    out << "Expected Stochastic Cost: " << sol.expected_cost << endl;
    out << "Open Facilities Count: " << sol.num_open_facilities << endl;
    out << "--------------------------------------------" << endl;
    out << "Open Facilities Indices:" << endl;
    for (size_t i = 0; i < sol.openFacilities.size(); ++i) {
        if (sol.openFacilities[i]) {
            out << i << " ";
        }
    }
    out << endl;
    out << "--------------------------------------------" << endl;
    out << "Client Assignments (ClientIndex -> FacilityIndex):" << endl;
    for (size_t j = 0; j < sol.assigned_facility.size(); ++j) {
        out << j << " -> " << sol.assigned_facility[j].first << "\n";
    }
}

void Writer::saveParameters(const GAParams& ga, const SAParams& sa, const string& filename) {
    ofstream out(filename);
    if (!out.is_open()) return;

    out << "=== GA PARAMETERS ===" << endl;
    out << "Pop Size: " << ga.pop_size << endl;
    out << "Max Generations: " << ga.max_generations << endl;
    out << "Mutation Rate: " << ga.mutation_rate << endl;
    out << "Elite Count: " << ga.elite_count << endl;
    out << "Use Local Search: " << ga.use_local_search << endl;
    out << "Use Smart Leader: " << ga.use_smart_leader << endl;
    out << "Seed: " << ga.seed << endl;
    
    out << "\n=== SA PARAMETERS ===" << endl;
    out << "Enabled: " << sa.solve << endl;
    out << "Initial Temp (T0): " << sa.T0 << endl;
    out << "Min Temp (Tmin): " << sa.Tmin << endl;
    out << "Alpha: " << sa.alpha << endl;
    out << "MC Samples (Search): " << sa.mc_samples << endl;
    out << "MC Penalty Factor (k): " << sa.mc_k << endl;
}

void Writer::createChart(const string& instance_name) {
    // Chama o script python via system call
    // Garanta que python3 está no path e o script existe
    string cmd = "python3 src/analysis/generateCharts.py " + instance_name;
    int res = system(cmd.c_str());
    (void)res; // Silencia warning de variável não usada
}

// Implementação legada/compatibilidade se necessária
void Writer::saveGaStats(const std::string& instance_name, const GaRunMetrics& metrics, const std::string& subfolder) {
    // Redireciona para a nova lógica baseada em Instance Name se necessário
    // Mas como o Solver.cpp já faz a iteração manual, esta função pode ficar vazia ou ser removida no futuro.
}
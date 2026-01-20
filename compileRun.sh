#!/usr/bin/env bash
set -euo pipefail

# --- CONFIGURATION MENU ---
echo "================================================"
echo "          UFLP SOLVER CONFIGURATION            "
echo "================================================"

if [[ -n "${1-}" ]]; then
    MODE_OPT="1"
    SINGLE_FILE="$1"
else
    echo "Select Run Mode:"
    echo "  1) Single Instance (Interactive)"
    echo "  2) GA Analysis Only (Interactive Params)"
    echo "  3) STRATEGY COMPARISON (Batch: LS vs SL vs None)"
    echo "  4) PAPER BENCHMARK (Replicate Table 6 & Boxplots)"
    echo "  5) STATISTICAL ROBUSTNESS (30 Runs Analysis)"
    read -p "Option: " MODE_OPT
fi

GA_ONLY_FLAG=""
LS_FLAG=""
SL_FLAG=""
BATCH_STRATEGY="false"
PAPER_BENCHMARK="false"
STATS_FLAG="false"
RUN_SINGLE="false"

if [[ "$MODE_OPT" == "5" ]]; then
    STATS_FLAG="true"
elif [[ "$MODE_OPT" == "4" ]]; then
    PAPER_BENCHMARK="true"
elif [[ "$MODE_OPT" == "3" ]]; then
    BATCH_STRATEGY="true"
elif [[ "$MODE_OPT" == "2" ]]; then
    GA_ONLY_FLAG="--ga-only"
    read -p "  Use Local Search? (0/1) [1]: " LS_IN
    LS_IN=${LS_IN:-1}
    read -p "  Use Smart Leader? (0/1) [1]: " SL_IN
    SL_IN=${SL_IN:-1}
    LS_FLAG="--ls $LS_IN"
    SL_FLAG="--sl $SL_IN"
    if [[ -z "${SINGLE_FILE-}" ]]; then RUN_SINGLE="true"; fi
elif [[ "$MODE_OPT" == "1" ]]; then
    if [[ -z "${SINGLE_FILE-}" ]]; then RUN_SINGLE="true"; fi
else
    echo "Invalid Option."
    exit 1
fi

# --- COMPILATION ---
echo -e "\n[Building Project...]"
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build -j"$(nproc || echo 1)"

EXEC="./build/uflp"
if [[ ! -f "$EXEC" ]]; then
    echo "Error: Compilation failed. Executable not found."
    exit 1
fi

INSTANCES_DIR="instancias_MED"
INSTANCES_ABS_PATH="$(realpath "$INSTANCES_DIR")"

if [[ ! -d "$INSTANCES_ABS_PATH" ]]; then
    echo "Error: Instances directory '$INSTANCES_DIR' not found!"
    exit 1
fi

# --- FUNCTIONS ---
# Função auxiliar para salvar os dados da estratégia atual antes que o C++ os apague
save_strategy_data() {
    local strat_name="$1"
    echo "  -> Backing up results for '$strat_name'..."
    
    # Para cada pasta de instância gerada em results/
    for inst_dir in results/*; do
        if [[ -d "$inst_dir" ]]; then
            inst_name=$(basename "$inst_dir")
            
            # Cria a pasta destino no cofre (results_safe)
            # Ex: results_safe/500-10/LS-OFF_SL-OFF/
            target_dir="results_safe/$inst_name/$strat_name"
            mkdir -p "$target_dir"
            
            # Move e renomeia o CSV para o formato que o Python espera
            if [[ -f "$inst_dir/history_ga.csv" ]]; then
                cp "$inst_dir/history_ga.csv" "$target_dir/ga_metrics.csv"
            fi
        fi
    done
}

# --- EXECUTION LOGIC ---

# 1. MODO ESTATÍSTICO
if [[ "$STATS_FLAG" == "true" ]]; then
    if [[ ! -f "src/analysis/plot_stats_30runs.py" ]]; then
        echo "Error: Python script not found!"
        exit 1
    fi
    python3 src/analysis/plot_stats_30runs.py --run-cpp "$EXEC" --dir "$INSTANCES_ABS_PATH"
    exit 0
fi

# 2. MODO PAPER BENCHMARK
if [[ "$PAPER_BENCHMARK" == "true" ]]; then
    python3 src/analysis/run_paper_experiment.py
    exit 0
fi

# 3. MODO STRATEGY BATCH (CORRIGIDO)
if [[ "$BATCH_STRATEGY" == "true" ]]; then
    echo -e "\n>>> STARTING STRATEGY COMPARISON BATCH <<<"
    
    # Prepara o "Cofre" temporário
    rm -rf results_safe
    mkdir -p results_safe
    
    echo "Running GA on ALL instances with 4 configurations..."
    
    # 1. No LS, No SL
    echo -e "\n[1/4] Running: LS: OFF | SL: OFF"
    "$EXEC" -all "$INSTANCES_ABS_PATH" --ga-only --ls 0 --sl 0 > /dev/null
    save_strategy_data "LS-OFF_SL-OFF"

    # 2. LS Only
    echo -e "\n[2/4] Running: LS: ON  | SL: OFF"
    "$EXEC" -all "$INSTANCES_ABS_PATH" --ga-only --ls 1 --sl 0 > /dev/null
    save_strategy_data "LS-ON_SL-OFF"

    # 3. SL Only
    echo -e "\n[3/4] Running: LS: OFF | SL: ON"
    "$EXEC" -all "$INSTANCES_ABS_PATH" --ga-only --ls 0 --sl 1 > /dev/null
    save_strategy_data "LS-OFF_SL-ON"

    # 4. Full (LS + SL)
    echo -e "\n[4/4] Running: LS: ON  | SL: ON"
    "$EXEC" -all "$INSTANCES_ABS_PATH" --ga-only --ls 1 --sl 1 > /dev/null
    save_strategy_data "LS-ON_SL-ON"
    
    # Restaura os dados para a pasta results/ para o Python ler
    echo -e "\nRestoring consolidated data..."
    rm -rf results
    mv results_safe results
    
    # Gera Gráficos
    echo "Generating comparative charts..."
    if [[ -f "src/analysis/compare_strategies.py" ]]; then
        python3 src/analysis/compare_strategies.py
        echo "Analysis charts generated in 'results/comparative_analysis/'."
    else
        echo "Warning: Python script not found."
    fi
    exit 0
fi

# 4. MODO INTERATIVO
if [[ "$RUN_SINGLE" == "true" || -n "${SINGLE_FILE-}" ]]; then
    if [[ -z "${SINGLE_FILE-}" ]]; then
        options=("$INSTANCES_ABS_PATH"/*.txt)
        if [[ ${#options[@]} -eq 0 ]]; then echo "No files."; exit 1; fi
        echo -e "\nAvailable Instances:"
        for i in "${!options[@]}"; do
            printf "%3d) %s\n" "$((i+1))" "$(basename "${options[$i]}")"
        done
        read -p "Select instance (0 for ALL): " sel
        if [[ "$sel" == "0" ]]; then
            "$EXEC" -all "$INSTANCES_ABS_PATH" $GA_ONLY_FLAG $LS_FLAG $SL_FLAG
            exit 0
        else
            SELECTED_FILE="${options[$((sel-1))]}"
        fi
    else
        SELECTED_FILE="$SINGLE_FILE"
    fi
    echo -e "\nRunning Solver on $(basename "$SELECTED_FILE")..."
    "$EXEC" -i "$SELECTED_FILE" $GA_ONLY_FLAG $LS_FLAG $SL_FLAG
fi
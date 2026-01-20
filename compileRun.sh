#!/usr/bin/env bash
set -euo pipefail

# --- CONFIGURATION MENU ---
echo "================================================"
echo "          UFLP SOLVER CONFIGURATION            "
echo "================================================"

# Verificação inicial de argumentos
if [[ -n "${1-}" ]]; then
    # Se passar arquivo direto, assume modo Single Run
    MODE_OPT="1"
    SINGLE_FILE="$1"
else
    echo "Select Run Mode:"
    echo "  1) Single Instance (Interactive)"
    echo "  2) GA Analysis Only (Interactive Params)"
    echo "  3) STRATEGY COMPARISON (Batch: LS vs SL vs None)"
    echo "  4) PAPER BENCHMARK (Replicate Table 6 & Boxplots)"
    echo "  5) STATISTICAL ROBUSTNESS (30 Runs Analysis)" # <--- NOVA OPÇÃO
    read -p "Option: " MODE_OPT
fi

# Variáveis de Controle
GA_ONLY_FLAG=""
LS_FLAG=""
SL_FLAG=""
BATCH_STRATEGY="false"
PAPER_BENCHMARK="false"
STATS_FLAG="false" # <--- NOVA FLAG
RUN_SINGLE="false"

# Lógica de Seleção
if [[ "$MODE_OPT" == "5" ]]; then
    echo -e "\n[STATISTICAL MODE SELECTED]"
    echo "Running 30 executions per instance to analyze variance and robustness."
    echo "This may take a long time."
    STATS_FLAG="true"

elif [[ "$MODE_OPT" == "4" ]]; then
    echo -e "\n[PAPER BENCHMARK SELECTED]"
    echo "This will run the experiment loop (k=5, 10, 20) to replicate paper results."
    PAPER_BENCHMARK="true"

elif [[ "$MODE_OPT" == "3" ]]; then
    echo -e "\n[STRATEGY COMPARISON BATCH SELECTED]"
    echo "This will run GA configurations on all instances to compare strategies."
    BATCH_STRATEGY="true"

elif [[ "$MODE_OPT" == "2" ]]; then
    GA_ONLY_FLAG="--ga-only"
    echo -e "\n[GA ANALYSIS SELECTED]"
    echo "Parameters for Genetic Algorithm:"
    read -p "  Use Local Search? (0/1) [1]: " LS_IN
    LS_IN=${LS_IN:-1}
    read -p "  Use Smart Leader? (0/1) [1]: " SL_IN
    SL_IN=${SL_IN:-1}
    
    LS_FLAG="--ls $LS_IN"
    SL_FLAG="--sl $SL_IN"
    
    if [[ -z "${SINGLE_FILE-}" ]]; then
        RUN_SINGLE="true" # Vai perguntar arquivo depois
    fi

elif [[ "$MODE_OPT" == "1" ]]; then
    echo -e "\n[SINGLE RUN SELECTED]"
    if [[ -z "${SINGLE_FILE-}" ]]; then
        RUN_SINGLE="true"
    fi

else
    echo "Invalid Option."
    exit 1
fi

# --- COMPILATION ---
echo -e "\n[Building Project...]"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

EXEC="./build/uflp"
if [[ ! -f "$EXEC" ]]; then
    echo "Error: Compilation failed. Executable not found."
    exit 1
fi

# --- PATH CONFIG ---
# Diretório onde as instâncias estão (ajuste se necessário)
INSTANCES_DIR="instancias_MED"
INSTANCES_ABS_PATH="$(pwd)/$INSTANCES_DIR"

# --- EXECUTION LOGIC ---

# MODO ESTATÍSTICO (NOVO)
if [[ "$STATS_FLAG" == "true" ]]; then
    # Verifica se o script python existe
    if [[ ! -f "src/analysis/plot_stats_30runs.py" ]]; then
        echo "Error: Python script src/analysis/plot_stats_30runs.py not found!"
        exit 1
    fi
    
    echo "Launching Python Statistical Controller..."
    python3 src/analysis/plot_stats_30runs.py --run-cpp "$EXEC" --dir "$INSTANCES_ABS_PATH"
    
    echo -e "\nCheck results in results/stats_plots/"
    exit 0
fi

# MODO PAPER BENCHMARK
if [[ "$PAPER_BENCHMARK" == "true" ]]; then
    echo "Launching Python Experiment Controller..."
    python3 src/analysis/run_paper_experiment.py
    exit 0
fi

# MODO STRATEGY BATCH
if [[ "$BATCH_STRATEGY" == "true" ]]; then
    echo "Launching Python Strategy Comparison..."
    python3 src/analysis/compare_strategies.py
    exit 0
fi

# MODO INTERATIVO (SINGLE OU BATCH NA PASTA)
if [[ "$RUN_SINGLE" == "true" || -n "${SINGLE_FILE-}" ]]; then
    
    if [[ -z "${SINGLE_FILE-}" ]]; then
        if [[ ! -d "$INSTANCES_ABS_PATH" ]]; then
            echo "Error: Directory '$INSTANCES_DIR' not found."
            exit 1
        fi

        # Lista arquivos
        options=("$INSTANCES_ABS_PATH"/*.txt)
        if [[ ${#options[@]} -eq 0 ]]; then
            echo "No instance files found in $INSTANCES_DIR"
            exit 1
        fi

        echo -e "\nAvailable Instances:"
        echo "------------------------------------------------"
        for i in "${!options[@]}"; do
            filename=$(basename "${options[$i]}")
            printf "%3d) %s\n" "$((i+1))" "$filename"
        done
        echo "------------------------------------------------"
        
        read -p "Select instance number (or 0 for ALL in folder): " sel
        
        if [[ "$sel" == "0" ]]; then
            # Roda na pasta toda com as configs selecionadas
            echo "Running on ALL instances in $INSTANCES_DIR..."
            "$EXEC" -all "$INSTANCES_ABS_PATH" $GA_ONLY_FLAG $LS_FLAG $SL_FLAG
            exit 0
        elif [[ "$sel" =~ ^[0-9]+$ ]] && (( sel >= 1 && sel <= ${#options[@]} )); then
            SELECTED_FILE="${options[$((sel-1))]}"
        else
            echo "Invalid selection."
            exit 1
        fi
    else
        SELECTED_FILE="$SINGLE_FILE"
    fi

    # Executa Única Instância
    echo -e "\nRunning Solver on $(basename "$SELECTED_FILE")..."
    "$EXEC" -i "$SELECTED_FILE" $GA_ONLY_FLAG $LS_FLAG $SL_FLAG
fi
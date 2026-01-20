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
    read -p "Option: " MODE_OPT
fi

# Variáveis de Controle
GA_ONLY_FLAG=""
LS_FLAG=""
SL_FLAG=""
BATCH_STRATEGY="false"
PAPER_BENCHMARK="false"
RUN_SINGLE="false"

# Lógica de Seleção
if [[ "$MODE_OPT" == "4" ]]; then
    echo -e "\n[PAPER BENCHMARK SELECTED]"
    echo "This will run the experiment loop (k=5, 10, 20) to replicate paper results."
    PAPER_BENCHMARK="true"

elif [[ "$MODE_OPT" == "3" ]]; then
    echo -e "\n[STRATEGY COMPARISON BATCH SELECTED]"
    echo "This will run GA configurations on all instances to compare strategies."
    BATCH_STRATEGY="true"

elif [[ "$MODE_OPT" == "2" ]]; then
    GA_ONLY_FLAG="--ga-only"
    echo -e "\n[GA CONFIGURATION]"
    
    # 2. Local Search
    read -p "  > Enable Local Search in GA? (y/n): " ls_opt
    if [[ "$ls_opt" == "y" || "$ls_opt" == "Y" ]]; then
        LS_FLAG="--ls 1"
    else
        LS_FLAG="--ls 0"
    fi

    # 3. Smart Leader
    read -p "  > Enable Smart Leader? (y/n): " sl_opt
    if [[ "$sl_opt" == "y" || "$sl_opt" == "Y" ]]; then
        SL_FLAG="--sl 1"
    else
        SL_FLAG="--sl 0"
    fi
    RUN_SINGLE="true"

elif [[ "$MODE_OPT" == "1" ]]; then
    echo -e "\n[FULL PIPELINE SELECTED]"
    RUN_SINGLE="true"
    # Default params for full pipeline
    LS_FLAG="--ls 1"
    SL_FLAG="--sl 1"

else
    echo "Invalid option."
    exit 1
fi

# --- BUILD STEP ---
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INSTANCES_DIR="instancias_MED"
INSTANCES_ABS_PATH="$(realpath "$ROOT_DIR/$INSTANCES_DIR")"

echo -e "\n>>> Building Project..."
mkdir -p "$BUILD_DIR"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build "$BUILD_DIR" -j"$(nproc || echo 1)"

EXEC="$BUILD_DIR/uflp"
if [[ ! -x "$EXEC" ]]; then
    echo "Error: Executable not found at $EXEC"
    exit 1
fi

# --- EXECUTION LOGIC ---

# CASO 1: PAPER BENCHMARK (Novo)
if [[ "$PAPER_BENCHMARK" == "true" ]]; then
    SCRIPT_PY="$ROOT_DIR/src/analysis/run_paper_experiment.py"
    
    if [[ ! -f "$SCRIPT_PY" ]]; then
        echo "Error: Experiment script not found at $SCRIPT_PY"
        echo "Please create it using the Python code provided previously."
        exit 1
    fi
    
    echo -e "\n>>> STARTING PAPER REPLICATION EXPERIMENT <<<"
    echo "Executing Python script wrapper..."
    echo "This may take a while as it runs for k=5, 10, and 20."
    
    python3 "$SCRIPT_PY"
    
    echo -e "\nDone! Check 'results/paper_replication/' for Tables and Charts."
    exit 0
fi

# CASO 2: STRATEGY COMPARISON BATCH
if [[ "$BATCH_STRATEGY" == "true" ]]; then
    echo -e "\n>>> STARTING STRATEGY COMPARISON BATCH <<<"
    
    # 1. No LS, No SL
    echo -e "\n[1/4] Running: GA Only | LS: OFF | SL: OFF"
    "$EXEC" -all "$INSTANCES_ABS_PATH" --ga-only --ls 0 --sl 0

    # 2. LS Only
    echo -e "\n[2/4] Running: GA Only | LS: ON  | SL: OFF"
    "$EXEC" -all "$INSTANCES_ABS_PATH" --ga-only --ls 1 --sl 0

    # 3. SL Only
    echo -e "\n[3/4] Running: GA Only | LS: OFF | SL: ON"
    "$EXEC" -all "$INSTANCES_ABS_PATH" --ga-only --ls 0 --sl 1

    # 4. Full
    echo -e "\n[4/4] Running: GA Only | LS: ON  | SL: ON"
    "$EXEC" -all "$INSTANCES_ABS_PATH" --ga-only --ls 1 --sl 1
    
    echo "Batch run complete."
    exit 0
fi

# CASO 3: SINGLE / INTERACTIVE MODE
if [[ "$RUN_SINGLE" == "true" ]]; then
    # Se arquivo não foi passado via argumento, abre seletor
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
    echo -e "\nRunning Solver on: $(basename "$SELECTED_FILE")"
    "$EXEC" -i "$SELECTED_FILE" $GA_ONLY_FLAG $LS_FLAG $SL_FLAG
fi
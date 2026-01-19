#!/usr/bin/env bash
set -euo pipefail

# --- CONFIGURATION MENU ---
echo "================================================"
echo "          UFLP SOLVER CONFIGURATION            "
echo "================================================"

# 1. Choose Mode
echo "Select Run Mode:"
echo "  1) Full Pipeline (GA + SA) - Single/All Instances"
echo "  2) GA Analysis Only (Interactive Params)"
echo "  3) BATCH EXPERIMENT (Test ALL configs on ALL instances)"
read -p "Option: " mode_opt

# Default Variables
GA_ONLY_FLAG=""
LS_FLAG=""
SL_FLAG=""
BATCH_MODE="false"

if [[ "$mode_opt" == "3" ]]; then
    echo -e "\n[BATCH MODE SELECTED]"
    echo "This will run the GA on all instances with 4 configurations:"
    echo "1. No LS, No Smart Leader"
    echo "2. LS Only"
    echo "3. Smart Leader Only"
    echo "4. Full (LS + Smart Leader)"
    BATCH_MODE="true"

elif [[ "$mode_opt" == "2" ]]; then
    GA_ONLY_FLAG="--ga-only"
    echo -e "\n[GA CONFIGURATION]"
    
    # 2. Local Search
    read -p "  > Enable Local Search in GA? (y/n): " ls_opt
    if [[ "$ls_opt" == "y" || "$ls_opt" == "Y" ]]; then
        LS_FLAG="--ls 1"
        echo "    -> Local Search: ON"
    else
        LS_FLAG="--ls 0"
        echo "    -> Local Search: OFF"
    fi

    # 3. Smart Leader
    read -p "  > Enable Smart Leader (Greedy Init)? (y/n): " sl_opt
    if [[ "$sl_opt" == "y" || "$sl_opt" == "Y" ]]; then
        SL_FLAG="--sl 1"
        echo "    -> Smart Leader: ON"
    else
        SL_FLAG="--sl 0"
        echo "    -> Smart Leader: OFF"
    fi
else
    # Default for full pipeline (usually both ON)
    echo "-> Running Full Pipeline (Default Config: GA+LS+SL -> SA)"
    LS_FLAG="--ls 1"
    SL_FLAG="--sl 1"
fi

echo "================================================"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INSTANCES_DIR="instancias_MED" 
INSTANCES_ABS_PATH="$(realpath "$ROOT_DIR/$INSTANCES_DIR")"

ARGS=()

# --- INSTANCE SELECTION LOGIC (Skipped if Batch Mode) ---
if [[ "$BATCH_MODE" == "false" ]]; then
    
    # Check if arguments were passed directly to script
    if [[ -n "${1-}" ]]; then
        INSTANCE_FILE="$1"
        if [[ -f "$INSTANCE_FILE" ]]; then
            FULL_PATH="$(realpath "$INSTANCE_FILE")"
        elif [[ -f "$ROOT_DIR/$INSTANCE_FILE" ]]; then
            FULL_PATH="$(realpath "$ROOT_DIR/$INSTANCE_FILE")"
        else
            echo "Error: Instance file '$INSTANCE_FILE' not found."
            exit 1
        fi
        ARGS=("-i" "$FULL_PATH")
    else
        # Interactive Selection
        if [[ ! -d "$ROOT_DIR/$INSTANCES_DIR" ]]; then
            echo "Error: Directory '$INSTANCES_DIR' not found."
            exit 1
        fi

        options=("$ROOT_DIR/$INSTANCES_DIR"/*)
        if [ ${#options[@]} -eq 0 ]; then
            echo "No instances found in $INSTANCES_DIR"
            exit 1
        fi

        echo "Available Instances:"
        for i in "${!options[@]}"; do
            filename=$(basename "${options[$i]}")
            printf "%3d) %s\n" "$((i+1))" "$filename"
        done
        echo "------------------------------------------------"
        read -p "Select an option number (0 for ALL): " selection

        if [[ "$selection" == "0" ]]; then
            ARGS=("-all" "$INSTANCES_ABS_PATH")
        elif [[ "$selection" =~ ^[0-9]+$ ]] && (( selection >= 1 && selection <= ${#options[@]} )); then
            INSTANCE_FILE="${options[$((selection-1))]}"
            FULL_PATH="$(realpath "$INSTANCE_FILE")"
            ARGS=("-i" "$FULL_PATH")
        else
            echo "Invalid selection."
            exit 1
        fi
    fi
fi

# --- BUILD ---
echo "Building project..."
mkdir -p "$BUILD_DIR"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build "$BUILD_DIR" -j"$(nproc || echo 1)"

EXEC="$BUILD_DIR/uflp"

# --- RUN LOGIC ---

if [[ "$BATCH_MODE" == "true" ]]; then
    # --- BATCH EXECUTION LOOP ---
    echo ">>> STARTING BATCH EXPERIMENT <<<"
    echo "Target Directory: $INSTANCES_ABS_PATH"

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

    echo -e "\n>>> BATCH EXPERIMENT COMPLETED SUCCESSFULLY <<<"
    echo "You can now run: python3 src/analysis/compare_strategies.py"

else
    # --- SINGLE/STANDARD EXECUTION ---
    echo "Running: $EXEC ${ARGS[@]} $GA_ONLY_FLAG $LS_FLAG $SL_FLAG"
    "$EXEC" "${ARGS[@]}" $GA_ONLY_FLAG $LS_FLAG $SL_FLAG
fi
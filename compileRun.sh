#!/usr/bin/env bash
set -euo pipefail

# Function to display usage
usage() {
  echo "Usage: $(basename "$0") [INSTANCE_FILE]"
  echo "If INSTANCE_FILE is omitted, an interactive menu will appear."
  exit 1
}

# Check for help flags
if [[ "${1-}" == "-h" || "${1-}" == "--help" ]]; then
  usage
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INSTANCES_DIR="instancias_MED" 

# Array to hold arguments for the C++ executable
ARGS=()

# --- SELECTION LOGIC ---

# 1. If an argument is provided, use it as a single instance file.
if [[ -n "${1-}" ]]; then
    INSTANCE_FILE="$1"
    
    # Resolve path
    if [[ -f "$INSTANCE_FILE" ]]; then
        FULL_PATH="$(realpath "$INSTANCE_FILE")"
    elif [[ -f "$ROOT_DIR/$INSTANCE_FILE" ]]; then
        FULL_PATH="$(realpath "$ROOT_DIR/$INSTANCE_FILE")"
    else
        echo "Error: Instance file '$INSTANCE_FILE' not found."
        exit 1
    fi
    
    # Set args for Single Instance Mode
    ARGS=("-i" "$FULL_PATH")

# 2. If no argument, show interactive menu.
else
    if [[ ! -d "$ROOT_DIR/$INSTANCES_DIR" ]]; then
        echo "Error: Directory '$INSTANCES_DIR' not found."
        exit 1
    fi

    echo "Scanning for instances in $INSTANCES_DIR..."
    
    # Get all .txt files
    mapfile -t options < <(find "$ROOT_DIR/$INSTANCES_DIR" -maxdepth 1 -name "*.txt" | sort)

    if [[ ${#options[@]} -eq 0 ]]; then
        echo "No .txt instance files found."
        exit 1
    fi

    echo "------------------------------------------------"
    echo " Available Options:"
    echo "------------------------------------------------"
    echo "  0) [BATCH] Run ALL instances in sequence"
    echo "------------------------------------------------"
    
    # List files with numbers
    for i in "${!options[@]}"; do
        filename=$(basename "${options[$i]}")
        printf "%3d) %s\n" "$((i+1))" "$filename"
    done
    echo "------------------------------------------------"

    # Prompt user
    read -p "Select an option number: " selection

    # Handle Selection
    if [[ "$selection" == "0" ]]; then
        # Batch Mode
        DIR_PATH="$(realpath "$ROOT_DIR/$INSTANCES_DIR")"
        ARGS=("-all" "$DIR_PATH")
        echo "Selected: Run ALL (Batch Mode)"
        
    elif [[ "$selection" =~ ^[0-9]+$ ]] && (( selection >= 1 && selection <= ${#options[@]} )); then
        # Single Instance Mode
        INSTANCE_FILE="${options[$((selection-1))]}"
        FULL_PATH="$(realpath "$INSTANCE_FILE")"
        ARGS=("-i" "$FULL_PATH")
        echo "Selected: $(basename "$INSTANCE_FILE")"
        
    else
        echo "Invalid selection. Exiting."
        exit 1
    fi
    echo ""
fi

# --- BUILD & RUN LOGIC ---

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Configure and build (Quietly)
echo "Building..."
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build "$BUILD_DIR" -j"$(nproc || echo 1)"

# Run the executable
EXEC="$BUILD_DIR/uflp"
if [[ ! -x "$EXEC" ]]; then
  echo "Executable not found: $EXEC"
  exit 1
fi

echo "Running: $EXEC ${ARGS[*]}"
echo "------------------------------------------------"
echo ""

# Execute with the arguments array
"$EXEC" "${ARGS[@]}"
#!/usr/bin/env bash
set -euo pipefail

usage() {
  echo "Usage: $(basename "$0") [INSTANCE_FILE]"
  echo "Default INSTANCE_FILE: instancias_MED/500-10.txt"
  exit 1
}

if [[ "${1-}" == "-h" || "${1-}" == "--help" ]]; then
  usage
fi

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

INSTANCE_FILE="${1:-instancias_MED/500-10.txt}"

# Resolve instance path (supports absolute or repo-relative)
if [[ -f "$INSTANCE_FILE" ]]; then
  INSTANCE_PATH="$(realpath "$INSTANCE_FILE")"
elif [[ -f "$ROOT_DIR/$INSTANCE_FILE" ]]; then
  INSTANCE_PATH="$(realpath "$ROOT_DIR/$INSTANCE_FILE")"
else
  echo "Instance file not found: $INSTANCE_FILE"
  exit 1
fi

# Ensure build directory exists
mkdir -p "$BUILD_DIR"

# Configure and build (Release by default)
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j"$(nproc || echo 1)"

# Run the executable
EXEC="$BUILD_DIR/uflp"
if [[ ! -x "$EXEC" ]]; then
  echo "Executable not found: $EXEC"
  exit 1
fi

echo "Running: $EXEC $INSTANCE_PATH"
echo ""
echo ""
"$EXEC" "$INSTANCE_PATH"

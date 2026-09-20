#!/usr/bin/env bash
#
# Quick dev build for Linux/Xash3D.
# Configures (only if needed) and builds client.so + hl.so into ./build-dev.
#
# Usage:
#   ./build-dev.sh            # incremental build
#   ./build-dev.sh clean      # wipe build dir and reconfigure from scratch
#
# Extra cmake configure args can be passed through, e.g.:
#   ./build-dev.sh -- -DGAMEDIR=myrmod

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

BUILD_DIR="build-dev"
BUILD_TYPE="RelWithDebInfo"

if [[ "${1:-}" == "clean" ]]; then
	rm -rf "$BUILD_DIR"
	shift
fi

if [[ "${1:-}" == "--" ]]; then
	shift
fi

if [[ ! -d "$BUILD_DIR" ]]; then
	cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -B "$BUILD_DIR" -S . "$@"
fi

cmake --build "$BUILD_DIR" -j"$(nproc)"

echo
echo "Built libraries:"
find "$BUILD_DIR" -maxdepth 3 -name "*.so" -exec ls -la {} \;

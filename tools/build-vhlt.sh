#!/usr/bin/env bash
#
# Clones (if missing) and builds VHLT-V34 (Vluzacn's Half-Life Tools,
# TWHL-community mirror) - the map compiler suite (hlcsg, hlbsp, hlvis,
# hlrad, ripent) needed to compile .map sources into .bsp for this SDK's
# maps.
#
# Usage:
#   ./build-vhlt.sh            # clone (if missing), build
#   ./build-vhlt.sh clean      # clean build artifacts and rebuild
#   ./build-vhlt.sh update     # git pull before rebuilding

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

VHLT_REPO="https://github.com/twhl-community/VHLT-V34.git"
VHLT_DIR="VHLT-V34"
VHLT_SRC="$VHLT_DIR/src/zhlt-vluzacn"
PATCH_DIR="$(pwd)/../patches/vhlt"

if [[ ! -d "$VHLT_DIR" ]]; then
	git clone --recursive "$VHLT_REPO" "$VHLT_DIR"
elif [[ "${1:-}" == "update" ]]; then
	git -C "$VHLT_DIR" pull
	shift
fi

source "$(pwd)/lib/apply-patches.sh"

# Apply local patches (patches/vhlt/*.patch, *.diff) before building.
# Idempotent: a patch already applied (source tree reused between runs) is
# detected via --reverse --check and skipped instead of failing the build.
( cd "$VHLT_DIR" && apply_patches_from "$PATCH_DIR" )

cd "$VHLT_SRC"

if [[ "${1:-}" == "clean" ]]; then
	make clean
	shift
fi

make -j"$(nproc)"

echo
echo "VHLT tools built into: $(pwd)/bin"
ls -la bin/

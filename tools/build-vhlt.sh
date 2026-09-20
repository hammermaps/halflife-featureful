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

# Apply local patches (patches/vhlt/*.patch, *.diff) before building.
# Idempotent: a patch already applied (source tree reused between runs) is
# detected via --reverse --check and skipped instead of failing the build.
(
	cd "$VHLT_DIR"
	shopt -s nullglob
	patches=("$PATCH_DIR"/*.patch "$PATCH_DIR"/*.diff)
	shopt -u nullglob
	if [[ ${#patches[@]} -gt 0 ]]; then
		IFS=$'\n' patches=($(sort <<<"${patches[*]}")); unset IFS
		echo "Applying patches from $PATCH_DIR"
		for p in "${patches[@]}"; do
			if git apply --check "$p" 2>/dev/null; then
				git apply "$p"
				echo "  applied: $(basename "$p")"
			elif git apply --reverse --check "$p" 2>/dev/null; then
				echo "  already applied, skipping: $(basename "$p")"
			else
				echo "  FAILED to apply: $(basename "$p")" >&2
				exit 1
			fi
		done
	fi
)

cd "$VHLT_SRC"

if [[ "${1:-}" == "clean" ]]; then
	make clean
	shift
fi

make -j"$(nproc)"

echo
echo "VHLT tools built into: $(pwd)/bin"
ls -la bin/

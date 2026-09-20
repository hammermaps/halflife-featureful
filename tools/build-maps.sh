#!/usr/bin/env bash
#
# Compiles every .map source in mod/featureful/maps/*.map into a .bsp,
# in place, using the VHLT-V34 tools (hlcsg -> hlbsp -> hlvis -> hlrad).
#
# Usage:
#   ./build-maps.sh                # fast dev compile (fast vis, sparse rad, no bounce)
#   ./build-maps.sh full           # full-quality compile (full vis, extra rad, bounces)
#   ./build-maps.sh lightonly      # entities/lighting-only re-run (see below), needs an
#                                   # existing .bsp from a prior fast/full build
#   ./build-maps.sh <name>         # compile only mod/featureful/maps/<name>.map
#   ./build-maps.sh full <name>    # combine both
#
# Run ./build-vhlt.sh first to build the compiler binaries.
#
# hlrad runs with -incremental (fast/full/lightonly alike), which caches the
# patch transfer matrix in a <mapname>.inc file next to the .bsp and reuses
# it on the next compile, skipping the most expensive step of the rad pass
# (GatherLight's patch visibility computation). This only actually saves
# anything when radiosity bounces run at all - the "fast" preset uses
# -bounce 0, which skips the transfer/vismatrix step entirely, so .inc is
# never written or read there; only "full" and "lightonly" benefit.
#
# patches/vhlt/0001-hlrad-incremental-geometry-checksum.patch makes the
# cache detect a geometry change that keeps the same patch count (e.g. a
# brush moved or rotated in place) instead of silently reusing a stale
# transfer matrix - see that patch's header comment for details.
#
# IMPORTANT: hlcsg deletes any existing .inc on every *normal* run
# (common/log.cpp: ResetTmpFiles(), skipped only for -onlyents). So under
# the default "fast"/"full" full-chain compile, the .inc cache never
# survives to the next build-maps.sh invocation anyway - it only pays off
# within "lightonly", which runs hlcsg -onlyents (entity/keyvalue changes
# only, e.g. tuning a light entity's brightness) instead of a full
# recompile, so ResetTmpFiles never fires and the cache from the last
# fast/full build is reused. "lightonly" does NOT pick up brush/geometry
# edits - re-run "fast" or "full" for those.

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

ROOT="$(cd .. && pwd)"
VHLT_BIN="$(pwd)/VHLT-V34/src/zhlt-vluzacn/bin"
VHLT_TOOLS="$(pwd)/VHLT-V34/tools"
MAPS_DIR="$ROOT/mod/featureful/maps"
VALVE_DIR="$ROOT/mod/valve"

for tool in hlcsg hlbsp hlvis hlrad; do
	if [[ ! -x "$VHLT_BIN/$tool" ]]; then
		echo "$tool not found in $VHLT_BIN. Run ./build-vhlt.sh first." >&2
		exit 1
	fi
done

if [[ ! -d "$MAPS_DIR" ]]; then
	echo "Maps directory not found: $MAPS_DIR" >&2
	exit 1
fi

# The .map files embed absolute wad paths from whatever machine authored them
# (e.g. /home/someone/.../Half-Life/valve/halflife.wad). WADROOT makes hlcsg
# fall back to "$WADROOT/<last-dir-of-original-path>/<wadfile>" when the
# literal path doesn't exist, which resolves against mod/valve here since
# the original paths all end in ".../valve/<wadfile>".
export WADROOT="$ROOT/mod"

# zhlt.wad isn't part of a normal Half-Life install; the maps expect it
# alongside the other wads (see readme.txt in tools/VHLT-V34), so make sure
# it's there for the WADROOT fallback to find.
mkdir -p "$VALVE_DIR"
if [[ ! -f "$VALVE_DIR/zhlt.wad" ]]; then
	cp "$VHLT_TOOLS/zhlt.wad" "$VALVE_DIR/zhlt.wad"
fi

quality="fast"
if [[ "${1:-}" == "full" || "${1:-}" == "lightonly" ]]; then
	quality="$1"
	shift
fi

shopt -s nullglob
if [[ $# -gt 0 ]]; then
	maps=("$MAPS_DIR/$1.map")
	if [[ ! -f "${maps[0]}" ]]; then
		echo "Map not found: ${maps[0]}" >&2
		exit 1
	fi
else
	maps=("$MAPS_DIR"/*.map)
fi
shopt -u nullglob

if [[ ${#maps[@]} -eq 0 ]]; then
	echo "No .map files found in $MAPS_DIR" >&2
	exit 1
fi

for mapfile in "${maps[@]}"; do
	mapname="${mapfile%.map}"
	echo "=== Compiling $(basename "$mapfile") ($quality) ==="

	if [[ "$quality" == "lightonly" ]]; then
		if [[ ! -f "$mapname.bsp" ]]; then
			echo "$(basename "$mapname").bsp not found - run a full/fast build first." >&2
			exit 1
		fi
		"$VHLT_BIN/hlcsg" -wadautodetect -onlyents "$mapname"
		"$VHLT_BIN/hlrad" -extra -bounce 8 -vismatrix sparse -incremental -lights "$VHLT_TOOLS/lights.rad" "$mapname"
	else
		"$VHLT_BIN/hlcsg" -wadautodetect "$mapname"
		"$VHLT_BIN/hlbsp" "$mapname"

		if [[ "$quality" == "full" ]]; then
			"$VHLT_BIN/hlvis" -full "$mapname"
			"$VHLT_BIN/hlrad" -extra -bounce 8 -vismatrix sparse -incremental -lights "$VHLT_TOOLS/lights.rad" "$mapname"
		else
			"$VHLT_BIN/hlvis" -fast "$mapname"
			# No -incremental here: -bounce 0 skips hlrad's transfer/vismatrix
			# step entirely, so there would be nothing to cache.
			"$VHLT_BIN/hlrad" -bounce 0 -vismatrix sparse -lights "$VHLT_TOOLS/lights.rad" "$mapname"
		fi
	fi

	echo "=== Done: $(basename "$mapname").bsp ==="
done

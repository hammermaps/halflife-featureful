#!/usr/bin/env bash
#
# Regression test for patches/vhlt/0001-hlrad-incremental-geometry-checksum.patch.
#
# Verifies that hlrad's -incremental transfer cache (<mapname>.inc):
#   1. is accepted as-is on an unmodified re-run (no full recompute), and
#   2. is correctly rejected as stale when its geometry checksum bytes are
#      corrupted, even though the patch count is untouched - this is
#      exactly the class of bug the patch fixes (a brush move that keeps
#      the same patch count used to be silently accepted pre-patch).
#
# Uses the small, self-contained, copyright-free sealed test room in
# tools/testdata/ci_test_room.map (single NULL-textured room, zhlt.wad
# only) rather than any private mod/ content, so this runs unmodified on
# a fresh checkout / in CI. Run after ./build-vhlt.sh.
#
# Usage: ./test-hlrad-incremental.sh

set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

VHLT_BIN="$(pwd)/VHLT-V34/src/zhlt-vluzacn/bin"
VHLT_TOOLS="$(pwd)/VHLT-V34/tools"
SRC_MAP="$(pwd)/testdata/ci_test_room.map"

for tool in hlcsg hlbsp hlvis hlrad; do
	if [[ ! -x "$VHLT_BIN/$tool" ]]; then
		echo "$tool not found in $VHLT_BIN. Run ./build-vhlt.sh first." >&2
		exit 1
	fi
done

if [[ ! -f "$SRC_MAP" ]]; then
	echo "Test map not found: $SRC_MAP" >&2
	exit 1
fi

WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT
MAP="$WORKDIR/ci_test_room"

# The template references zhlt.wad (bundled with the VHLT tools, not a
# copyrighted game asset) via a placeholder substituted with its real path.
sed "s|@ZHLT_WAD@|$VHLT_TOOLS/zhlt.wad|" "$SRC_MAP" > "$MAP.map"

fail() {
	echo "FAIL: $1" >&2
	exit 1
}

echo "=== Building a fresh baseline (csg -> bsp -> vis -> rad -incremental) ==="
"$VHLT_BIN/hlcsg" -wadautodetect "$MAP" >/dev/null
"$VHLT_BIN/hlbsp" "$MAP" >/dev/null
[[ ! -f "$MAP.pts" ]] || fail "test room map leaks (found $MAP.pts) - fix tools/testdata/ci_test_room.map"
"$VHLT_BIN/hlvis" -fast "$MAP" >/dev/null
"$VHLT_BIN/hlrad" -extra -bounce 8 -vismatrix sparse -incremental -lights "$VHLT_TOOLS/lights.rad" "$MAP" >"$WORKDIR/hlrad_baseline.log" 2>&1

[[ -f "$MAP.inc" ]] || fail "baseline run did not produce $MAP.inc"
grep -q "Writing transfers file" "$WORKDIR/hlrad_baseline.log" || fail "baseline run did not write the transfer cache"
echo "OK: baseline .inc created"

echo "=== Re-running hlrad unchanged: cache must be accepted ==="
"$VHLT_BIN/hlrad" -extra -bounce 8 -vismatrix sparse -incremental -lights "$VHLT_TOOLS/lights.rad" "$MAP" >"$WORKDIR/hlrad_valid.log" 2>&1

grep -q "Reading transfers file" "$WORKDIR/hlrad_valid.log" || fail "valid re-run did not attempt to read the cache"
grep -q "Finished reading transfers file" "$WORKDIR/hlrad_valid.log" || fail "valid, unmodified cache was rejected (false positive)"
echo "OK: unmodified cache accepted"

echo "=== Corrupting the stored checksum (patch count untouched) ==="
# total_patches (a `long`) is written first, immediately followed by the
# 8-byte geometry checksum. Flip the checksum's last byte only - sizeof(long)
# depends on the build (4 on a 32-bit build, 8 on 64-bit), so compute the
# offset instead of assuming one.
long_bytes=$(( $(getconf LONG_BIT) / 8 ))
checksum_offset=$(( long_bytes + 7 ))
python3 - "$MAP.inc" "$checksum_offset" <<'PYEOF'
import sys
path, offset = sys.argv[1], int(sys.argv[2])
with open(path, "r+b") as f:
    f.seek(offset)
    b = f.read(1)
    f.seek(offset)
    f.write(bytes([b[0] ^ 0xFF]))
PYEOF

echo "=== Re-running hlrad with a corrupted cache: must be rejected as stale ==="
"$VHLT_BIN/hlrad" -extra -bounce 8 -vismatrix sparse -incremental -lights "$VHLT_TOOLS/lights.rad" "$MAP" >"$WORKDIR/hlrad_stale.log" 2>&1

grep -q "is stale (geometry changed), recomputing" "$WORKDIR/hlrad_stale.log" || fail "corrupted checksum was NOT detected - regression in the geometry checksum patch!"
grep -q "Writing transfers file" "$WORKDIR/hlrad_stale.log" || fail "stale cache was detected but not recomputed/rewritten"
echo "OK: corrupted checksum correctly rejected and cache rebuilt"

echo
echo "=== All checks passed ==="

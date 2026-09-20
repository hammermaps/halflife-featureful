# patches/vhlt

Patch files applied to the freshly cloned `tools/VHLT-V34` checkout before
it's built by `tools/build-vhlt.sh`.

## Format

- One `git apply`-compatible patch per file, extension `.patch` or `.diff`.
- Patches are applied in alphabetical order — prefix with a number if order
  matters, e.g. `0001-fix-something.patch`, `0002-other-fix.patch`.
- Generate a patch from inside the `tools/VHLT-V34` checkout with:
  ```
  git diff > ../patches/vhlt/0001-my-change.patch
  ```

## Behavior

`build-vhlt.sh` applies every patch here right after cloning (or updating)
`VHLT-V34`, before running `make`. Patches that are already applied (e.g.
because the source tree is reused between runs) are detected and skipped
instead of failing the build.

## Patches

- `0001-hlrad-incremental-geometry-checksum.patch` — adds a geometry
  checksum (FNV-1a over each patch's origin/area/face and full winding
  point list) to hlrad's `-incremental` transfer cache (`.inc` file).
  Without this, the cache is only invalidated when the *patch count*
  changes; a brush move (or in-place rotation around its own centroid)
  that happens to keep the same patch count is silently accepted and
  produces stale bounce lighting. See `hlrad/transfers.cpp`'s
  `GeometryChecksum()`. Regression-tested by
  `tools/test-hlrad-incremental.sh`.

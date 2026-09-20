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

- `0002-cleanup-freeblock-null-warning-progress-throttle-ub.patch` —
  three small, independent cleanups found while reviewing the codebase
  for modernization opportunities (all low-risk, log/warning-hygiene or
  pure UB fixes, no behavior change to compiled output):
  - `common/threads.cpp: GetThreadWork()` printed `\r%6d /%6d` on
    *every single* dispatched work item while holding the global work
    mutex, for every patch/face/etc. — the direct cause of the
    enormous (1.8MB+ for one `hlrad -extra -bounce 8` run), mostly
    single-line compile logs we kept running into while testing patch
    0001. Now gated behind the same `f != oldf` percentage-bucket check
    the time-estimate print right below it already uses.
  - `common/blockmem.cpp: FreeBlock()` (both the `SYSTEM_WIN32` and
    `SYSTEM_POSIX` implementations) logged `Warning("Freeing a null
    pointer")` for a null pointer, which is normal, expected behavior
    (same as `free(NULL)`) — this produced ~28,000 spurious warning
    lines in a single `-incremental` `FailedRead` cleanup pass during
    our own regression testing. Now a silent no-op, matching `free()`.
  - `common/bspfile.cpp`'s texinfo byte-swap loop walked
    `vecs[0][0..7]` (`vecs` is `float[2][4]`), reading 4 floats past
    the end of `vecs[0]` into `vecs[1]`'s memory — undefined behavior
    per the standard, and the exact cause of the
    `-Waggressive-loop-optimizations: iteration 4 invokes undefined
    behavior` warning we saw building this project. Split into two
    explicit loops over `vecs[0]` and `vecs[1]`; behaviorally identical
    on every real compiler, just removes the UB.

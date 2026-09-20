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

- `0003-posix-auto-detect-thread-count.patch` — this VHLT-V34 fork's
  POSIX `ThreadSetDefault()` (`common/threads.cpp`) always fell back to
  1 thread unless `-threads` was passed explicitly: `common/threads.h`
  hardcoded `DEFAULT_NUMTHREADS 1` for `SYSTEM_POSIX`, so the (already
  present but unreachable) auto-detect branch in `ThreadSetDefault()`
  was dead code on Linux — every `hlcsg`/`hlbsp`/`hlvis`/`hlrad` run
  silently used a single core unless a caller passed `-threads`
  explicitly (e.g. J.A.C.K.'s bundled Build Programs configs don't).
  Ports the working auto-detection from
  [`github.com/FreeSlave/vhlt`](https://github.com/FreeSlave/vhlt) (a
  different personal fork of the same Vluzacn ZHLT v34 base):
  `sched_getaffinity`/`CPU_COUNT` on Linux, `sysctlbyname` on
  macOS/BSD, `sysconf(_SC_NPROCESSORS_ONLN)` as the generic Unix
  fallback. Sets `DEFAULT_NUMTHREADS -1` ("not set manually") for
  POSIX, matching the Windows side, which already auto-detected via
  `GetSystemInfo()`. Verified: `hlcsg`/`hlrad` report `threads [ 8 ]`
  with no `-threads` flag on an 8-core machine; `hlrad -extra -bounce 8`
  on the demo map dropped from ~51s to ~12s once actually using all
  cores.

- `0004-double-lightmap-resolution-xash3d.patch` — doubles lightmap
  resolution (8 texels per luxel instead of 16) for every face,
  unconditionally: `common/bspfile.h`'s `TEXTURE_STEP`/
  `MAX_SURFACE_EXTENT` change from `16`/`16` to `8`/`64` (the same pair
  of values stock VHLT's own dormant `ZHLT_XASH2` build flag already
  used — doubling `MAX_SURFACE_EXTENT` alongside halving `TEXTURE_STEP`
  keeps the maximum physical face size before lightmap subdivision the
  same or larger, rather than forcing more/smaller lightmap patches),
  and `hlcsg/textures.cpp` sets a new `TEX_EXTRA_LIGHTMAP` texinfo flag
  bit (matching Xash3D FWGS's `common/bspfile.h: TEX_EXTRA_LIGHTMAP`,
  `BIT(3)`) on every non-`TEX_SPECIAL` face so the engine actually
  samples at the finer step.
  **Deliberately does *not* use `ZHLT_XASH2`** — that flag also bumps
  `BSPVERSION` to `31`, which this project's actual `xash3d-fwgs`
  (`engine/common/mod_bmodel.c: Mod_LoadBmodelLumps`) doesn't recognize
  and would reject outright (`case HLBSP_VERSION`/`Q1BSP_VERSION`/
  `QBSP2_VERSION` only, `default:` errors). `TEX_EXTRA_LIGHTMAP` is a
  per-texinfo flag bit the engine reads independently of the file's
  BSP version, so this stays on an ordinary `BSPVERSION 30` file, fully
  compatible with this project's engine build. Since this project only
  targets Xash3D FWGS (never classic GoldSource/Software renderer/HLDS,
  which is what the removed `16`/`16` comment warned about), the change
  is unconditional rather than gated behind a build flag. Verified via
  gdb-wrapped `hlcsg`/`hlbsp`/`hlvis`/`hlrad` runs on the demo map (no
  crash) and visually in-game (sharper lighting, no lightmap artifacts
  or corruption).

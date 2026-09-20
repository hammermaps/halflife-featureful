# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

Half-Life Featureful SDK — a fork/superset of the classic GoldSource Half-Life SDK (`dlls` = server game logic, `cl_dll` = client game logic) targeting both GoldSource and Xash3D FWGS engines, with extra monster/weapon/entity features and a data-driven (JSON) configuration layer on top of the original C++ entity system. It's a game DLL project (`server.dll`/`client.dll` equivalents), not an application — there is no "run" step other than loading the built libraries into a Half-Life-compatible engine.

## Build

Two parallel build systems exist: **CMake** (primary, used in CI) and **waf** (legacy, still supported).

### CMake

```bash
cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
cmake --build build
```

Useful options (pass as `-D<NAME>=<VALUE>`, see `mod_options.txt` for the full list of mod-specific options):
- `BUILD_CLIENT` / `BUILD_SERVER` (default `ON`) — build only one side.
- `64BIT` — 32-bit build is the default on x86 platforms; most Half-Life engines expect 32-bit.
- `GOLDSOURCE_SUPPORT` — build a client lib compatible with original GoldSource (auto-detected per platform).
- `GAMEDIR` + `CMAKE_INSTALL_PREFIX` — set to install straight into a mod directory, then build with `--target install`.
- Custom mod options can be added to `mod_options.txt` as `NAME=VALUE # description` and are auto-registered as CMake options and `-D` compile definitions.

Outputs land as `hl.dll`/`client.dll` (Windows) or `.so` equivalents (Linux) under `build/dlls` and `build/cl_dll`.

### waf (legacy, requires Python 2.7+)

```bash
./waf configure -T release
./waf
```

### Platform notes
- Linux target is normally a 32-bit Steam Runtime build (see README for the podman/schroot/chroot recipes); a plain `cmake --build` on a modern distro also works but binaries may not be portable to old Steam runtimes.
- Android: open `android/` in Android Studio, or `cd android && ./gradlew assembleRelease`.
- Also supports Nintendo Switch (devkitPro) and PS Vita (VitaSDK) via both waf and CMake.

## Tests

Unit tests live in `tests/` and use GoogleTest, built as a **separate CMake project** (not part of the top-level `CMakeLists.txt` build):

```bash
cd tests
cmake -B build -S .
cmake --build build
./build/test
```

Requires `libgtest-dev` (or gtest discoverable by CMake) installed on the system. Tests compile a curated subset of `game_shared/`, `dlls/`, `cl_dll/`, and `public/` sources directly (see `tests/CMakeLists.txt`'s explicit source list) alongside the `tests/*_test.cpp` files — the game code isn't built as a library first. To run a single test binary filter, use gtest's own flag: `./build/test --gtest_filter=SuiteName.CaseName`.

When adding a new `dlls`/`cl_dll`/`game_shared` source file that a test needs, add it to the `add_executable(test ...)` source list in `tests/CMakeLists.txt` explicitly.

## Architecture

### Client/server split (classic GoldSource model)
- `dlls/` — server-side game logic (monsters, weapons, entities, game rules). Compiled with `-DSERVER_DLL` into the server game DLL.
- `cl_dll/` — client-side game logic (HUD, view effects, client-side prediction, VGUI). Compiled with `-DCLIENT_DLL` into the client DLL.
- `game_shared/` — code compiled into *both* DLLs (shared entity data, weapon parameter tables, JSON config parsing, damage info, materials, sound scripts, etc.).
- `pm_shared/` — the player movement code, shared between client (prediction) and server (authoritative movement).
- `engine/`, `common/`, `public/` — engine-facing interfaces/headers (`eiface.h`, `cdll_int.h`, etc.) supplied by the engine (GoldSource or Xash3D); these are mostly headers this project doesn't own, defining the ABI boundary.
- `freevgui/` — git submodule, the FWGS VGUI reimplementation used for the client UI on non-GoldSource-compatible platforms.

### Data-driven configuration layer
A large part of "Featureful" behavior is controlled by JSON configs loaded at runtime rather than hardcoded in C++, parsed via `game_shared/json_config.*` / `json_utils.*` and schema helpers (`schema_definitions.*`, `schema_weapons.*`). Examples: weapon templates/parameters (`weapon_templates.*`, `weapon_parameters.*`), sound scripts (`soundscripts.*`), entity templates (`ent_templates.*` in `dlls/`), warpball templates (`warpball.*`), object hints, materials, skill/difficulty data, save titles, followers, inventory. Each of these has a matching doc page under `documentation/content/docs/configuration/`. When changing behavior that's exposed through one of these systems, check the corresponding doc page and keep it in sync, and check `tests/` for an existing `*_test.cpp` covering it — most of these subsystems have dedicated GoogleTest coverage.

### Mod configuration vs. engine configuration
`mod_options.txt` defines compile-time mod options (surfaced as CMake options/`-D` defines). `features/*.cfg` are runtime engine config files (exec'd cfgs for weapons, monsters, HUD layout, ammo, etc.) — don't confuse the two; the former is a build-time toggle list, the latter is data consumed by the running mod.

### Documentation site
`documentation/` is a Hugo site (theme via git submodule `documentation/themes/hugo-book`) published to the project's GitHub Pages. `documentation/content/docs/configuration/` and `entity-guide/` are the primary reference for user/mapper-facing behavior — consult them when a change affects documented behavior, and update them alongside the code.

### CI
`.github/workflows/build.yml` builds client+server for Linux (32-bit, via Steam Runtime schroot) and Windows on every push/PR, then runs the `tests/` GoogleTest suite on Linux only. `manual*.yml` workflows handle on-demand/Android builds; `hugo.yml` deploys the documentation site.

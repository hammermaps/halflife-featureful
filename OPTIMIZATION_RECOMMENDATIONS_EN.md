# Optimization Opportunities for Half-Life Featureful SDK

This documentation lists possible optimizations for the Half-Life Featureful SDK project, categorized by different areas.

> **Last updated**: April 2026 – automatically updated based on current codebase analysis.
>
> **Note**: Line number references may change over time. Use them as a guide, but verify against current code.

### Current Project Statistics

| Metric | Value |
|--------|-------|
| Total source files (.cpp/.h/.c) | 594 |
| Lines of Code (excl. external/) | ~270,000 |
| C++ Standard | C++11 (C++14 for some targets) |
| TODO markers | 94 |
| FIXME markers | 27 |
| HACK/HACKHACK markers | 62 |
| BUGBUG markers | 31 |
| Total code markers | 214 (excl. DEBUG references) |
| Unsafe `strcpy` calls | 106 |
| Unsafe `strcat` calls | 59 |
| Unsafe `sprintf` calls | 126 |
| Total unsafe string operations | 291 |
| Raw `malloc`/`calloc`/`realloc` (excl. external/) | 42 |
| Headers with `#pragma once` | 226 |
| Headers without `#pragma once` | 40 |
| Unit tests | 16 test files in `tests/` |

## 1. Build System Optimizations

### 1.1 Compiler Optimizations
- **Link-Time Optimization (LTO)**: Already available as option (`-DLTO=ON`), but disabled by default. Could be enabled by default for Release builds
  - Benefit: 5-15% performance improvement, smaller binaries
  - Location: `CMakeLists.txt` line ~50, ~186-193
  
- **Polly Optimizations**: Already available as option (`-DPOLLY=ON`), but disabled by default
  - Benefit: Advanced loop optimizations for better performance
  - Location: `CMakeLists.txt` line ~51, ~224-230
  
- **Extended Compiler Flags**:
  - `-O3` for maximum optimization instead of `-O2`
  - `-march=native` for CPU-specific optimizations (for local builds)
  - `-ffast-math` for faster math operations (if precision isn't critical)
  - `-funroll-loops` for loop unrolling

### 1.2 Unity Builds
- **Implement Unity/Jumbo Builds**: Compile multiple .cpp files in one translation unit
  - Benefit: Faster compilation (up to 50% faster), better optimization opportunities
  - CMake supports this with `UNITY_BUILD` property
  - Location: `cl_dll/CMakeLists.txt`, `dlls/CMakeLists.txt`

### 1.3 Precompiled Headers (PCH)
- **Introduce PCH for common headers**: Precompile frequently used headers like `extdll.h`, `util.h`, `cbase.h`
  - Benefit: Significantly faster compilation times (30-50% improvement)
  - Location: `dlls/` and `cl_dll/` directories

### 1.4 Caching
- **Enable ccache or sccache**: Build caching for faster recompilation
  - Benefit: 10x faster builds for small changes
  - Configure in CMake with `CMAKE_C_COMPILER_LAUNCHER` and `CMAKE_CXX_COMPILER_LAUNCHER`

## 2. Code Quality and Security

### 2.1 Replace Unsafe String Operations
- **Problem**: 291 occurrences of unsafe string functions
  - `strcpy`: 106 calls (58 in `dlls/`, 43 in `cl_dll/`, 1 in `pm_shared/`, 1 in `game_shared/`, 3 in `utils/`)
  - `strcat`: 59 calls (39 in `dlls/`, 15 in `cl_dll/`, 3 in `game_shared/`)
  - `sprintf`: 126 calls (45 in `dlls/`, 78 in `cl_dll/`, 3 in `utils/`)
  - Replace with: `strncpy`, `strncat`, `snprintf`, or better: `strlcpy`, `strlcat` (already available)
  - Benefit: Prevent buffer overflow vulnerabilities
  - Location: Distributed across all `.cpp` files

### 2.2 Modernize Memory Management
- **Replace raw malloc/calloc/realloc**: 42 occurrences in the codebase (excl. external/)
  - Mainly in: `dlls/nodes.cpp` (26), `dlls/world.cpp` (1), `dlls/visuals.cpp` (1), `game_shared/` (2), `cl_dll/` (4)
  - Replace with: C++ `new`/`delete` or better: Smart Pointers (`std::unique_ptr`, `std::shared_ptr`)
  - Benefit: Automatic memory management, no memory leaks

### 2.3 Enable Compiler Warnings
- **Activate more warning flags**: Currently some are commented out
  - Enable `-Wall`, `-Wextra`, `-Wpedantic`
  - Enable `-Werror=format=2` (commented out in wscript)
  - Benefit: Early detection of bugs
  - Location: `wscript` lines 154-187

### 2.4 Static Code Analysis
- **Clang-Tidy Integration**: Add static code analysis
  - Detection of: Memory leaks, uninitialized variables, dead code
  - CMake integration with `CMAKE_CXX_CLANG_TIDY`
  
- **Cppcheck Integration**: Additional analysis tool
  - Parallel to Clang-Tidy for better coverage

## 3. Performance Optimizations

### 3.1 Profiling Infrastructure
- **Add profiling hooks**: Time measurement for critical code paths
  - Tools: gprof, valgrind, perf
  - Macro system for easy enable/disable

### 3.2 Algorithm Optimizations
- **Optimize string operations**: String comparisons and conversions
  - Replace `strcmp` with hash comparisons where possible
  - String pools for frequently used strings

- **Optimize math operations**:
  - SIMD instructions for vector/matrix operations
  - Lookup tables for trigonometric functions
  - Fast inverse square root for normalization

### 3.3 Memory Layout
- **Cache-friendly data structures**: Array of Structures (AoS) to Structure of Arrays (SoA)
  - Better CPU cache utilization
  - Especially important for entity/monster arrays

- **Memory Pooling**: Object pools for frequently allocated objects
  - Reduces fragmentation and allocation time
  - Especially for projectiles, particles, temporary entities

### 3.4 Virtual Function Optimization
- **Reduce virtual function overhead**: 800+ virtual functions in 80+ files
  - Devirtualization where possible
  - `final` keyword for leaf classes (e.g., specific monster classes, weapons)
  - Location: Especially `dlls/gamerules.h` (69), `dlls/weapons.h` (42), `dlls/cbase.h` (105), `dlls/basemonster.h` (84)

## 4. Parallelization

### 4.1 Multithreading Opportunities
- **Asynchronous operations**:
  - Asset loading in background threads
  - Parallelize physics calculations
  - Offload sound processing

- **Implement thread pool**: For recurring parallel tasks
  - Standard: `std::thread`, `std::async`, or thread pool library

### 4.2 Build Parallelization
- **Increase parallel build jobs**: Default to `-j$(nproc)` in build instructions
  - Benefit: Faster builds on multi-core systems
  - Location: README.md build instructions

## 5. Platform-Specific Optimizations

### 5.1 SIMD Optimizations
- **SSE/AVX for x86**: Accelerate vector operations
  - Compiler flags: `-msse4.2`, `-mavx`, `-mavx2`
  - Manual intrinsics for critical loops

- **NEON for ARM**: ARM-specific SIMD
  - Android and Nintendo Switch builds

### 5.2 Platform-Specific Code Paths
- **Optimized implementations**: Platform-optimized versions of critical functions
  - Conditional compilation with `#ifdef`
  - Runtime CPU feature detection

## 6. Asset Optimizations

### 6.1 Texture Optimizations
- **Compression**: DXT/BCn compression for textures
- **Mipmaps**: Automatic mipmap generation
- **Streaming**: Texture streaming for large levels

### 6.2 Model Optimizations
- **LOD System**: Level-of-Detail for models
- **Mesh optimization**: Vertex cache optimization
- **Bone reduction**: Fewer bones for distant characters

## 7. Documentation and Tooling

### 7.1 Performance Documentation
- **Create performance guide**: Best practices for mod developers
  - Profiling guide
  - Optimization patterns
  - Do's and Don'ts

### 7.2 Benchmark Suite
- **Automated benchmarks**: Performance regression tests
  - Frame time measurements
  - Physics performance
  - AI performance
  - Load times

### 7.3 Profiling Tools Integration
- **Tracy Profiler**: Real-time profiling with visualization
- **Chrome Tracing**: JSON-based profiling
- **Custom Profiler**: Simple timing framework

## 8. CI/CD Optimizations

### 8.1 Build Cache in CI
- **GitHub Actions Cache**: Cache dependencies and build artifacts
  - ccache for compiler cache
  - CMake build cache
  - Location: `.github/workflows/build.yml`

### 8.2 Matrix Build Optimization
- **Parallel builds**: Build different platforms in parallel
- **Conditional builds**: Build only on relevant changes
- **Artifact optimization**: Upload only necessary artifacts

### 8.3 Incremental Builds
- **Faster PR validation**: Build only changed components
- **Dependency tracking**: Intelligent rebuild decisions

## 9. Development Experience

### 9.1 Faster Iteration
- **Hot-reload**: Code changes without complete restart
- **Incremental linking**: Faster linking for small changes
- **Debug build optimization**: `-Og` instead of `-O0` for faster debug builds

### 9.2 Better Error Messages
- **Template error improvement**: Concepts (C++20) for better errors
- **Custom error messages**: `static_assert` with helpful messages
- **Compiler flags**: `-fdiagnostics-color=always` (already enabled)

### 9.3 Modern C++ Migration
- **C++17/20 features**: Range-based for loops, auto, constexpr
  - Current: C++11 (CMakeLists.txt line 23)
  - Upgrade to C++17 for better features
  - `std::optional`, `std::variant`, `std::string_view`

## 10. Testing and Quality Assurance

### 10.1 Test Coverage
- **Expand unit tests**: More tests in `tests/` directory
- **Integration tests**: End-to-end tests for critical features
- **Fuzzing**: Automated testing with random inputs

### 10.2 Memory Sanitizers
- **AddressSanitizer (ASan)**: Find memory errors
- **UndefinedBehaviorSanitizer (UBSan)**: Detect undefined behavior
- **ThreadSanitizer (TSan)**: Find race conditions
- **MemorySanitizer (MSan)**: Uninitialized memory accesses

### 10.3 Continuous Benchmarking
- **Performance tracking**: Automatic performance measurement in CI
- **Regression detection**: Alerts on performance degradation
- **Historical comparison**: Performance trends over time

## 11. Code Organization

### 11.1 Header Optimization
- **Forward declarations**: Reduce include dependencies
- **Pimpl idiom**: Hide implementation details
- **Include guards**: 226 headers with `#pragma once`, but 40 headers still without – establish consistency

### 11.2 Module System (C++20)
- **Migration to modules**: Replace headers with modules
  - Significantly faster compilation
  - Better encapsulation
  - Future: C++20/23 modules

### 11.3 Dependency Management
- **Conan or vcpkg**: Modern dependency management
  - Easier management of external libraries
  - Location: `external/` directory

## 12. Specific Code Improvements

### 12.1 TODO/FIXME/HACK Analysis
- **214 code markers** (excl. DEBUG references): TODO (94), FIXME (27), HACK (62), BUGBUG (31)
  - Distribution: `dlls/` (128), `cl_dll/` (48), `pm_shared/` (8), `utils/` (29), `game_shared/` (1)
  - Systematic processing of these markers
  - Prioritization by importance
  - Details: See [goldsource.md](goldsource.md)

### 12.2 Dead Code Elimination
- **Remove unreachable code**: Use static analysis tools
- **Unused functions**: Enable compiler warnings (`-Wunused`)
- **Conditional code**: Remove obsolete #ifdef blocks

### 12.3 Const Correctness
- **Add const qualifiers**: Document immutability
- **Const member functions**: Mark where possible
- **Const references**: For large parameter objects

## 13. BG Particle System Optimizations

The BG particle system (ported from *Battle Grounds*) lives in `cl_dll/particles/`. The following system-specific optimizations were identified through direct code analysis.

### 13.1 OpenGL Rendering Efficiency

- **Replace immediate mode with vertex arrays / VBOs**: Each particle issues its own `glBegin(GL_QUADS)` / `glEnd()` pair. With hundreds of active particles this produces an equal number of draw calls.
  - Fix: Batch all particles sharing the same texture into a single `glBegin` block or VBO.
  - Location: `cl_dll/particles/particle.cpp:163-185` (CParticle::Draw), `particle.cpp:390-412` (CSparkParticle::Draw)
  - Benefit: Massive GPU throughput improvement – N draw calls reduced to 1 per texture

- **Compute view vectors once per frame**: `AngleVectors()` and `gEngfuncs.GetViewAngles()` are currently called once **per particle**, even though the camera direction does not change within a frame.
  - Fix: Compute `vForward`, `vRight`, `vUp` once in `UpdateSystems()` and pass them as parameters or frame-global variables.
  - Location: `cl_dll/particles/particle.cpp:147-156`
  - Benefit: Eliminates N×`AngleVectors` calls per frame (N = active particle count)

- **Texture-state batching**: Particles are sorted purely by distance, causing frequent `glBindTexture` switches. A two-level sort (texture first, distance tiers second) would reduce OpenGL state changes.
  - Location: `cl_dll/particles/system_manager.cpp:113-138`
  - Benefit: Fewer OpenGL state changes, better GPU cache utilization

- **Cache `flSize * flScale` locally**: In `CParticle::Draw()` the product `sParticle.flSize * sParticle.flScale` is recomputed for each of the 4 corners twice, yielding 8 multiplications per particle. A single local variable would cut this to 1.
  - Location: `cl_dll/particles/particle.cpp:166-183`

### 13.2 Data Structure Improvements

- **`vector::erase(begin()+i)` → swap-and-pop-back**: `RemoveParticle`, `RemoveParticles`, `RemoveSystems`, and `RemoveTextures` all use `erase(begin()+i)`, which shifts all subsequent elements (O(n) per removal). Since ordering does not matter during removal, swapping the target with the last element and calling `pop_back()` achieves O(1) removal.
  - Location: `cl_dll/particles/system_manager.cpp:566-679`
  - Benefit: Mass-removal complexity reduced from O(n²) to O(n)

- **Texture cache: `vector` → `std::unordered_map`**: `HasTexture()` performs a linear scan through a `vector<particle_texture_cache*>` using `stricmp`. This function is called every frame per active system inside `UpdateSystem`.
  - Fix: Replace with `std::unordered_map<std::string, particle_texture_s*>` using a lowercased key.
  - Location: `cl_dll/particles/system_manager.cpp:506-518`
  - Benefit: O(n) → O(1) texture lookup

- **Particle memory pool**: Every particle is individually heap-allocated with `new` and freed with `delete`. Explosions spawn and destroy many short-lived particles simultaneously.
  - Fix: A freelist / object pool for `CMappedParticle`, `CFlintParticle`, etc.
  - Location: `cl_dll/particles/system_manager.cpp:106`, `mapped_particles.cpp:106,113`
  - Benefit: Less heap fragmentation, faster allocation, better cache locality

### 13.3 Per-Frame Computation

- **`LoadTGA` called every frame**: `CMappedParticleSystem::UpdateSystem()` calls `LoadTGA(NULL, m_pSystem->sParticleTexture)` on every update. The result is already cached via `HasTexture()`, but the string scan still runs every frame.
  - Fix: Cache the texture pointer in the system object after the first load; only call `LoadTGA` during initialization.
  - Location: `cl_dll/particles/mapped_particles.cpp:97`
  - Benefit: Eliminates a `stricmp` scan per active system per frame

- **`fmod` instead of `while` loop for rotation wrap**: All `Update()` methods contain `while (sParticle.flCurrentRotation > 360) { flCurrentRotation -= 360; }`. Because rotation steps per frame are small, the loop body executes at most once in practice. An `if` or `fmod` is semantically clearer and marginally faster.
  - Location: `cl_dll/particles/particle.cpp:240-242, 337-339, 435-437, 528-530`

- **Cache `gEngfuncs.GetClientTime()` per frame**: `TimeSinceLastDraw()` and `DistanceToThisPlayer()` both call `gEngfuncs.GetClientTime()` internally. Caching this value once in `UpdateSystems()` and passing it as a parameter avoids redundant calls.
  - Location: `cl_dll/particles/system_manager.cpp:60`, `cl_dll/particles/particle.cpp:108`

### 13.4 Culling and LOD

- **Distance-based particle culling**: There is no maximum render distance for particles. Very distant particles are fully updated and drawn regardless of distance.
  - Fix: In `UpdateSystems()`, skip (or reduce update rate of) particles where `flSquareDistanceToPlayer` exceeds a configurable threshold.
  - Location: `cl_dll/particles/system_manager.cpp:86-139`

- **View frustum culling**: Particles outside the camera frustum are fully processed. A simple frustum AABB check before `Update()`/`Draw()` would skip invisible particles entirely.
  - Location: `cl_dll/particles/system_manager.cpp:86-139`

- **Leverage existing grass LOD fields**: `CGrassParticle` already has `m_flLodMinDistance` / `m_flLodMaxDistance` fields loaded from config, but they are not used to fully cull particles beyond `m_flLodMaxDistance`.
  - Location: `cl_dll/particles/grass_particle.cpp`

### 13.5 Memory Layout

- **Share system data for grass particles**: `CGrassParticleSystem` creates a full `grass_particle_system` copy for each individual grass particle via `new` + `memcpy`. Since most fields are identical across particles of the same type, using a shared const base pointer with per-particle deltas would significantly reduce memory usage.
  - Location: `cl_dll/particles/grass_system.cpp:65-66`
  - Benefit: Greatly reduced memory footprint for dense grass areas

- **Hot/cold split for `base_particle` struct**: The `base_particle` struct in `game_shared/particle_defs.h` mixes frequently-updated fields (position, velocity, age) with rarely-used ones (color components, texture pointer). Splitting them improves CPU cache efficiency during the update loop.
  - Location: `game_shared/particle_defs.h`

- **Store `GLuint` directly instead of as a pointer**: `particle_texture_s::iID` is declared as `GLuint*` – a pointer to a single integer stored on the heap. Storing `GLuint` directly in the struct eliminates one heap allocation and one level of indirection per texture.
  - Location: `cl_dll/particles/particle_texture.h:53`



### High Priority (Quick Wins)
1. Enable LTO for Release builds (easy, big impact)
2. Introduce precompiled headers (halve build time)
3. Replace unsafe string operations (security)
4. Implement CI build cache (faster CI)
5. Enable ccache (local build speed)

### Medium Priority
1. Implement Unity builds
2. Upgrade C++ to version 17
3. Enable more compiler warnings
4. Modernize memory management
5. Virtual function optimization

### Low Priority (Long-term)
1. SIMD optimizations
2. Threading optimizations
3. Module system (C++20)
4. Comprehensive benchmark suite
5. Modern dependency management

## Measurable Goals

### Build Time
- **Current**: ~2-5 minutes (full build)
- **Target**: <1 minute (with PCH, Unity builds, ccache)

### Performance
- **Current**: Baseline
- **Target**: 10-20% FPS improvement (LTO, optimized builds)

### Code Quality
- **Current**: 291 unsafe string operations, 214 code markers (TODO/FIXME/HACK/BUGBUG), 42 raw mallocs
- **Target**: 0 unsafe operations, 50% fewer code markers, no raw malloc

### Security
- **Current**: Potential buffer overflows
- **Target**: All known security vulnerabilities fixed

## Implementation Strategy

1. **Phase 1 (Quick Wins)**: Build system optimizations (1-2 weeks)
2. **Phase 2 (Security)**: String operations and memory management (2-3 weeks)
3. **Phase 3 (Performance)**: Algorithm optimizations and profiling (4-6 weeks)
4. **Phase 4 (Long-term)**: Threading, SIMD, modern C++ features (2-3 months)

## Resources and Tools

### Recommended Tools
- **Profilers**: Tracy, Valgrind, perf, VTune
- **Static Analysis**: Clang-Tidy, Cppcheck, PVS-Studio
- **Build**: Ninja (faster than Make), ccache/sccache
- **Testing**: Google Test, Catch2
- **CI/CD**: GitHub Actions (already present)

### Useful Libraries
- **Memory**: mimalloc, jemalloc (better allocators)
- **Containers**: abseil, folly (optimized containers)
- **Math**: GLM, Eigen (SIMD-optimized math)
- **Profiling**: Tracy, microprofile, Remotery

## Summary

This list contains over **65 concrete optimization opportunities** in 13 categories:
- **Build System**: 10+ optimizations
- **Code Quality**: 8+ improvements
- **Performance**: 15+ optimizations
- **Parallelization**: 5+ opportunities
- **Platform-specific**: 4+ optimizations
- **Assets**: 3+ optimizations
- **Tooling**: 6+ improvements
- **CI/CD**: 4+ optimizations
- **BG Particle System**: 15+ optimizations

Implementing all optimizations could lead to:
- **50-70% faster builds**
- **10-20% better runtime performance**
- **Significantly better code quality and security**
- **Better developer experience**

Focus should be on quick wins (LTO, PCH, build cache), followed by security improvements (string operations, memory management), and then performance optimizations.

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

## Prioritization

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

This list contains over **50 concrete optimization opportunities** in 12 categories:
- **Build System**: 10+ optimizations
- **Code Quality**: 8+ improvements
- **Performance**: 15+ optimizations
- **Parallelization**: 5+ opportunities
- **Platform-specific**: 4+ optimizations
- **Assets**: 3+ optimizations
- **Tooling**: 6+ improvements
- **CI/CD**: 4+ optimizations

Implementing all optimizations could lead to:
- **50-70% faster builds**
- **10-20% better runtime performance**
- **Significantly better code quality and security**
- **Better developer experience**

Focus should be on quick wins (LTO, PCH, build cache), followed by security improvements (string operations, memory management), and then performance optimizations.

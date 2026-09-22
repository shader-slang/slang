# Slice 198: Build and package the optional compiler-matched NVVM provider

This ExecPlan follows `.agent/PLANS.md`. The user explicitly requested committing completed slice
plans; that task-specific instruction overrides the normal working-log exclusion for this plan.

## Purpose and Observable Result

An opt-in normal Slang build produces its LLVM14 NVVM provider, stages it beside the compiler and
test executables, and includes it in installation and distribution archives. The default build
remains unchanged and never downloads LLVM14 implicitly.

## Progress

- [x] 2026-09-22: Inspect root and standalone CMake ownership and create this plan.
- [x] 2026-09-22: Prototype Debug/Release ExternalProject builds with Ninja Multi-Config.
- [x] 2026-09-22: Integrate optional build, staging, component install, and package component selection.
- [x] 2026-09-22: Validate disabled/invalid configuration, root build, clean normal package,
      relocation, exact-file loading and dependency rebuilding.
- [x] 2026-09-22: Complete the five-part report with input-shape audits and validation evidence.

## Surprises and Discoveries

ExternalProject initial directory creation does not expand configuration expressions. A dedicated
configuration-aware directory step must precede configure; this was observed and corrected in the
small harness before root integration.

The standalone provider already enforces exact LLVM14.0.6 static components and has platform-specific
export lists. The initial root CMake had no provider target. SlangTarget stages executables under config/bin,
with optional user output-directory overrides, so staging must use target directories.

## Decision Log

- 2026-09-22: Configure the standalone project with ExternalProject instead of add_subdirectory.
  LLVM14 imported target names must never share the root LLVM21 package namespace.
- 2026-09-22: Require an explicit existing SLANG_NVVM_LLVM_DIR and reject cross-compilation in this
  bounded native-build slice. Toolchain-native settings are propagated; host/target guesses are not.
- 2026-09-22: Keep one separate provider binary directory per configuration and invoke its build on
  each root build. Its own dependency graph decides whether sources or shared ABI headers changed.

- 2026-09-22: Deployment validation exposed an existing-file loader bug. The default shared-library
  loader now preserves actual platform files instead of appending a second platform suffix.
  The common loader owns this filesystem/path boundary; custom loaders retain their contracts.

## Outcomes and Retrospective

Prototype builds, custom executable-directory staging, component installation, unchanged incremental
staging, and ABI-header dependency tracking pass. Disabled configuration succeeds; missing-package
and cross-compilation configurations reject. Root enabled Debug build and focused gates pass (421 passed, 54 ignored). A normal binary package
extracted elsewhere compiled the motivating shader through adjacent/directory/exact loading at
O0/O3, and all six results assembled for sm_80. Missing/invalid overrides remained authoritative.
No GPU or native Windows/macOS execution is claimed.

Normal packaging revealed that the actual nonembedded runtime compiler and core cache were
classified as generators. The producer install-component assignment is now corrected; the final
ordinary package needs no generators-component workaround. Source/slang/CMakeLists.txt and the
provider README joined this slice ownership after lead approval.

## Context and Current Pipeline

For example, enabling the existing direct NVVM PTX mode loads slang-llvm-nvvm beside the executable.
The compiler's provider ABI header is shared with source/slang-llvm-nvvm, but users currently build
and copy that module manually. The root build should produce the same-checkout module through the
existing standalone CMake project, then stage and install that exact artifact.

## Scope and Non-Goals

Own cmake/NVVM.cmake, root CMakeLists.txt, CMakePresets.json, this plan, and its report. Also own source/core/slang-shared-library.cpp and unit-test-nvvm-builder.cpp for the discovered
exact-file deployment regression. No compiler semantic changes, LLVM downloads, GPU correctness
claims, or cross compilation. The provider README and source/slang/CMakeLists.txt also belong to
this slice after the packaging discoveries. Preserve standalone export/version validation.

## Architecture and Invariants

SLANG_ENABLE_NVVM_PROVIDER defaults OFF. Enabled builds require SLANG_NVVM_LLVM_DIR. ExternalProject
owns isolated provider configuration and incremental building; ordinary Slang executable targets
own their output directories. Staging copies the resulting module beside those executables.
Install and CPack use the slang-llvm-nvvm component in bin. Missing LLVM input is a configure error.

## Interfaces and Dependencies

Add optional SLANG_ENABLE_NVVM_PROVIDER and isolated package path SLANG_NVVM_LLVM_DIR. Use the root
generator, compilers, configuration, platform/toolset, and native toolchain settings in the child.
The child remains the single authority for LLVM14.0.6 static linking and exported ABI symbols.

## Milestones

1. Create a tiny local CMake harness under build/slice198-prototype, include cmake/NVVM.cmake, and
   verify Debug and Release artifacts with existing build/llvm14/lib/cmake/llvm.
2. Include the module after root executable targets exist; add provider target dependencies and
   component installation. Add component to binary package presets.
3. Validate default OFF and explicit invalid input; build/stage/install the provider. Compile a
   representative shader from a clean installed layout through the root driver's validation.

## Validation and Acceptance

Use native CMake. Prototype command shape: cmake -S <harness> -B <harness-build> -G "Ninja Multi-Config"
-DSLANG_ENABLE_NVVM_PROVIDER=ON -DSLANG_NVVM_LLVM_DIR=/opt/src/slang/build/llvm14/lib/cmake/llvm;
cmake --build <harness-build> --config Debug; cmake --install <harness-build> --config Debug
--prefix <stage> --component slang-llvm-nvvm. Require actual module output, preserved timestamp on
unchanged incremental rebuild, distinct configuration outputs, and rejection of missing LLVM path.
Root validation is coordinated with the lead so earlier slice builds are not reconfigured.

## Failure and Recovery

Disable SLANG_ENABLE_NVVM_PROVIDER to recover the established build. Existing isolated LLVM builds
are read-only dependencies and are never replaced. Keep prototype directories local under build.
Report unsupported platforms/configurations as explicit failures instead of selecting host tools.

## Artifacts and Hand-Off

Retain prototype commands and generated artifacts under build/slice198-prototype. Record root
build/install and compile validation in the five-part report and lead-owned durable design notes.

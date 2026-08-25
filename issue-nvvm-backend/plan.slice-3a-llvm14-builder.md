# Build an Isolated LLVM 14 NVVM Bitcode Module

This ExecPlan follows `.agent/PLANS.md`. It is an active working log under
`issue-nvvm-backend/` and must remain out of commits. Keep it current as work proceeds and distill
durable conclusions into `docs/design/nvvm-backend.md`.

## Purpose and Observable Result

This slice implements the writer boundary selected by Slice 2 without beginning general Slang IR
lowering. At completion, a separately configured optional module named `slang-llvm-nvvm` must
link a pinned LLVM 14.0.6 statically, export only a narrow versioned Slang C ABI, and construct an
empty NVVM IR 2.0 kernel as LLVM bitcode at runtime.

The shortest proof is one focused test process that:

1. loads the normal LLVM 21 `slang-llvm` module and the LLVM 14 `slang-llvm-nvvm` module;
2. queries the builder ABI and exact LLVM/NVVM IR versions;
3. uses general module/type/function/block/return/kernel-annotation operations to construct a
   caller-named empty kernel, proving the output is generated rather than a fixed checked-in blob;
4. forwards the returned bytes through `NVVMDownstreamCompiler`;
5. observes the requested `.visible .entry` in PTX and assembles that PTX with CUDA 12.2 `ptxas`;
   and
6. verifies the builder DLL imports no LLVM DLL and exports no raw LLVM symbols.

The established NVRTC route and the existing LLVM 21 CPU module remain unchanged. If a usable
LLVM 14 static development package cannot be established on this machine, the slice stops at a
restartable dependency/build prototype with exact diagnostics; it does not weaken symbol isolation
or substitute an opaque-pointer writer.

## Progress

- [x] (2026-08-25 22:26Z) Started repository, module-pattern, test-boundary, and local LLVM 14
  availability audits.
- [x] (2026-08-25 22:26Z) Confirmed the normal build uses a fetched LLVM 21 `slang-llvm.dll`, no
  LLVM 14 development package is installed on PATH, and the unrelated untracked
  `external/slang-binaries/` tree contains DXC binaries rather than LLVM.
- [x] (2026-08-26 00:18Z) Finalized a versioned pure-C function-table ABI with opaque handles,
  caller-owned two-call serialization, and the smallest general operations needed for an empty
  kernel; kept the LLVM 14 project in a separate CMake configure.
- [x] (2026-08-26 00:24Z) Shallow-cloned official LLVM tag `llvmorg-14.0.6` at commit
  `f28c006a5895fc0e329fe15fead81e37457cb1d1`, configured it with no targets/tools/projects/shared
  LLVM, and built the Release Core, BitWriter, and Support component targets under ignored
  `build/nvvm-builder-deps/`.
- [x] (2026-08-26 00:31Z) Built the isolated Release `slang-llvm-nvvm.dll`; `dumpbin /exports`
  reports only `slang_getNVVMBuilderAPI_V1`, while `/dependents` reports only Windows system DLLs
  and no LLVM DLL.
- [x] (2026-08-26 01:06Z) Implemented the host loader plus always-running fake ABI coverage and
  environment-aware real-module tests, including invalid-operation coverage against LLVM itself.
- [x] (2026-08-26 01:09Z) Loaded LLVM 21.1 before LLVM 14 in a fresh targeted test process,
  generated caller-named bitcode, compiled it through CUDA 12.2 libNVVM, and assembled the
  resulting PTX with CUDA 12.2 `ptxas -arch=sm_75`; all 25 focused NVVM tests pass.
- [x] (2026-08-26 01:11Z) Re-ran the unclosable-module, downstream-version, and ordinary NVRTC CUDA
  regressions; completed the helper/input-shape review, binary inspection, formatting, diff check,
  and durable design update.

## Surprises and Discoveries

- Observation: the main checkout is configured with Visual Studio 2022 and
  `SLANG_SLANG_LLVM_FLAVOR=FETCH_BINARY_IF_POSSIBLE`; `build/Debug/bin/slang-llvm.dll` is the
  fetched LLVM 21 module rather than a target built against system LLVM.
  Evidence: `build/CMakeCache.txt`, `slangc -llvm-version`, and the built binary directory.

- Observation: no `llvm-config.exe`, `llvm-as.exe`, LLVM CMake package, or LLVM 14 headers/libraries
  are installed in the probed system roots. `C:\LLVM` is a reduced LLVM 16.0.4 runtime with
  `LLVM-C.dll` but no general headers, static components, or CMake package, so it is not a viable
  producer dependency. The Visual Studio Ninja executable is available, but a separately configured
  LLVM build must still establish a Windows compiler environment.
  Evidence: PATH and filesystem probes recorded at slice start.

- Observation: immediately before Slang migrated its CPU plugin from LLVM 14.0.6 to LLVM 21.1.2,
  `external/build-llvm.ps1` and `.sh` pinned `llvmorg-14.0.6`, static MSVC runtimes, and disabled
  LLVM's C DLL, tools, tests, docs, examples, and benchmarks. That is strong acquisition/configure
  precedent, although this module needs only LLVM rather than Clang or target backends.
  Evidence: parent of commit `1f888a367`.

- Observation: LLVM's CMake documentation says `LLVM_BUILD_LLVM_DYLIB` is unavailable on Windows.
  A Windows source build therefore naturally supplies static component libraries, but the final
  module still needs an import/export audit because CMake configuration alone does not prove symbol
  isolation.
  Evidence: LLVM 14 CMake and Getting Started documentation.

- Observation: requesting the LLVM Core, BitWriter, and Support build targets caused LLVM's
  generated Visual Studio solution to build a broader static dependency closure, including
  `LLVMAnalysis`, even though the final module links only the component libraries returned for
  Core, BitWriter, and Support. The local source tree is about 1.14 GB and the component archives
  themselves are about 75 MB, 6.6 MB, and 31 MB respectively, so future packaging should use a
  prebuilt artifact rather than build LLVM in every ordinary Slang configure.
  Evidence: the successful component build log and produced Release archives under
  `build/nvvm-builder-deps/llvm-build/Release/lib/`.

- Observation: LLVM 14's `LLVM_DEFINITIONS` is one space-separated string in its generated CMake
  package on this platform. Passing it directly to `target_compile_definitions` creates one invalid
  MSVC macro. The standalone project must split it and pass the resulting `-D` arguments as compile
  options.
  Evidence: the first module build's C5102 diagnostic and the cleanly parsed regenerated project.

- Observation: Slang's public `slang.h` is a C++ interface and cannot make a private table header
  C-compatible merely because the new declarations themselves use C syntax. A real C translation
  unit failed immediately on scoped enums, `constexpr`, namespaces, and interfaces inherited from
  `slang.h`. The ABI header now owns its fixed-width result/classification types and local
  calling/export macros; native buffer and string counts remain `size_t`. The standalone build
  compiles a no-code C probe on every module build.
  Evidence: the first C-probe diagnostics and the subsequent clean C/C++ module build.

- Observation: the minimal builder API still has several ways to form incomplete IR, such as
  creating a block without a terminator. LLVM Core already contains the canonical module verifier,
  so serialization can reject this shape without linking another component or duplicating LLVM's
  validity rules.
  Evidence: `nvvmIRBuilderRejectsInvalidOperations` fails serialization for the unterminated module
  and succeeds after the same block receives `ret void`.

- Observation: the statically linked Release module is about 27 MB and has a one-symbol export
  table. Its only normal imports are `ADVAPI32.dll` and `KERNEL32.dll`, with delayed imports of
  `SHELL32.dll` and `ole32.dll`; there is no `LLVM.dll` or `LLVM-C.dll` dependency.
  Evidence: Visual Studio 2022 `dumpbin /exports` and `/dependents` on the produced DLL.

- Observation: CMake's `MODULE` library default suffix is `.so` on Darwin, while Slang's shared
  library loader constructs a `.dylib` filename. The standalone project must set `.dylib`
  explicitly on Apple platforms; Windows and ELF platforms already match the loader convention.
  Evidence: the final cross-platform review of `SharedLibrary::getPlatformFileName` and the module
  target properties.

- Observation: on 32-bit MSVC, `__stdcall` decorates the getter's internal COFF symbol, but an
  undecorated entry in a module-definition file resolves that internal symbol and exports only the
  caller-facing undecorated name. No architecture-specific alias is needed for the current MSVC
  build. Evidence: an x86 Visual Studio 2022 compile/link probe followed by `dumpbin /exports`.

Add evidence here as dependency, ABI, load, and isolation probes run. Move stable conclusions to
the Decision Log and design document.

## Decision Log

- Decision: make `slang-llvm-nvvm` a separately configured CMake project rather than call
  `find_package(LLVM 14)` from Slang's main configure.
  Rationale: one CMake configure cannot safely import LLVM 14 and LLVM 21 packages with the same
  target names, and the resulting module must be distributable independently like `slang-llvm`.
  Date/Author: 2026-08-25, Codex.
  Revisit when: CMake and LLVM provide a proven target-namespace isolation mechanism that also
  preserves runtime symbol isolation.

- Decision: name the module `slang-llvm-nvvm`.
  Rationale: the name describes the LLVM-to-NVVM producer role and already matches the
  `slang-llvm` prefix in `SharedLibrary::isUnclosable`, keeping the statically linked LLVM runtime
  resident after load without adding another lifecycle special case.
  Date/Author: 2026-08-25, Codex.
  Revisit when: the existing prefix lifetime rule becomes unnecessary or is replaced by explicit
  module ownership.

- Decision: expose one versioned C entry point returning a function table with opaque handles and
  caller-owned two-pass serialization buffers.
  Rationale: a C table avoids C++ ABI and LLVM-type leakage. Querying the required serialized size
  and then writing into caller storage prevents any allocation from crossing CRT/module ownership.
  Date/Author: 2026-08-25, Codex.
  Revisit when: the first real emitter operations reveal a missing lifetime or diagnostic concept.

- Decision: expose the smallest general operation set that can build the empty kernel instead of an
  `emitEmptyKernel` convenience operation.
  Rationale: module creation, void/function types, function declaration, blocks, `ret void`, kernel
  annotation, and serialization are independently useful lowering primitives. A caller-selected
  function name still proves runtime construction without retaining a test-only special case.
  Date/Author: 2026-08-25, Codex.
  Revisit when: a required primitive cannot be expressed without revising the V1 table.

- Decision: link only LLVM Core, BitWriter, and their required Support dependencies in this slice.
  Rationale: constructing LLVM IR and serializing bitcode needs no NVPTX code generator; libNVVM,
  not LLVM, owns PTX generation. A smaller static closure reduces build size and collision surface.
  Date/Author: 2026-08-25, Codex.
  Revisit when: a later measured lowering requirement needs another LLVM component.

- Decision: acquire the prototype dependency with a shallow Git clone of the exact upstream tag
  and record the resolved commit, instead of checking source or binaries into this repository.
  Rationale: the tag is the version already proven by Slice 2 and historical Slang builds, while
  the exact commit makes the ignored local prototype reproducible without defining the eventual
  packaging mechanism prematurely.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a CI/package slice defines signed binaries, hashes, and supported hosts.

- Decision: keep enum-like ABI values and result codes in fixed-width integer typedefs rather than
  pass C enums or `SlangResult` declarations from `slang.h` across the table.
  Rationale: C enum width can change with compiler flags and `slang.h` is C++-only. Fixed-width
  fields plus a signed-result convention make the table layout independently checkable from C.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a public stable ABI needs a generated compatibility manifest across architectures.

## Principled-Change Audit

The production helper/special-case inventory is intentionally small:

- `_hasRequiredFunctions` and `_validateHandleResult` validate the provider table and its required
  output postconditions at the dynamic-module boundary. They do not reconstruct LLVM or Slang IR.
- `_getModule`, `_getType`, `_getValue`, and `_getBlock` are the one conversion boundary from C
  opaque handles back to module-owned LLVM objects.
- `_getStringRef` is the one pointer-plus-count adapter; embedded NULs are preserved by the fake
  test rather than reinterpreted as C-string terminators.
- `_copySerializedData` is the one implementation of the caller-owned query/write protocol and
  assigns the required size before reporting an insufficient buffer.
- LLVM's existing `FunctionType::isValid*`, parent/context checks, symbol-table lookup, and
  `verifyModule` own type legality, cross-module ownership, duplicate-name rejection, and module
  structure respectively. No custom LLVM-equivalence or graph-walking fallback was introduced.

Consider this invalid-operation example: module A creates a void function and an entry block, while
module B supplies a type or receives A's function/block handle. The producing functions are
`_getVoidType`, `_declareFunction`, and `_createBlock`. Those handles are canonical and valid only
inside the LLVM context/module that produced them, so `_getFunctionType`, `_declareFunction`,
`_createBlock`, `_setInsertBlock`, and `_markFunctionAsKernel` reject the mismatched owner directly.
If A's block has no terminator, `verifyModule` rejects serialization at the producer boundary. The
real `nvvmIRBuilderRejectsInvalidOperations` test removes each guard in turn conceptually: without
the owner checks LLVM would receive a cross-context object, without the duplicate lookup LLVM would
silently rename the caller's symbol, and without the verifier malformed bitcode would reach
libNVVM. After `ret void` is emitted, the same module serializes successfully, proving the guard is
about the input shape rather than a test-specific fallback.

Rejected alternatives were extending LLVM 21's C++ `ILLVMBuilder`, returning an allocator-owned
Slang/LLVM blob across the module, embedding or parsing the Slice 2 fixture, registering a second
NVVM downstream compiler, accepting arbitrary stale/forged pointers through a global handle
registry, and duplicating LLVM's verifier with minimal-block checks. Each either puts the invariant
at the wrong layer, widens runtime collision/ownership risk, or creates a second source of truth.

## Outcomes and Retrospective

Slice 3a is complete on this machine. The exact LLVM 14.0.6 source builds without patches; the
separately configured C/C++ module links; and binary inspection proves the intended one-symbol,
no-LLVM-DLL boundary. The fake tests validate ABI rejection, caller-owned buffers, malformed output,
and lifetime. The real tests construct multiple caller-named kernels, reject malformed/cross-module
operations, compile LLVM 14 bitcode through CUDA 12.2 libNVVM, assemble it with `ptxas`, and re-query
the already-loaded LLVM 21.1 compiler afterward. The focused NVVM prefix passes 25/25.

The remaining limitations are explicit rather than hidden: packaging and the CI matrix are not
defined; coexistence currently proves the LLVM-21-first order but not both fresh-process load
orders; verifier text is intentionally discarded because V1 has no diagnostic channel; and every
serialize call verifies and materializes the module again. Those are inputs to later packaging,
diagnostics, and performance slices rather than reasons to widen this first builder ABI.

## Context and Current Pipeline

Slice 2 established this consumer path:

```text
checked-in LLVM 14 bitcode -> ObjectCode + LLVMIR + Kernel artifact
                           -> NVVMDownstreamCompiler -> libNVVM -> PTX -> ptxas
```

The fixture proves compatibility but is not a Slang-side writer. This slice adds only the producer
module and a host wrapper:

```text
general builder calls -> slang-llvm-nvvm (LLVM 14, separate DLL)
                      -> caller-owned bitcode buffer
                      -> host-owned Slang blob/artifact
                      -> NVVMDownstreamCompiler -> libNVVM -> PTX -> ptxas
```

`source/slang-llvm` demonstrates the optional-module pattern, but its `ILLVMBuilder` is a large C++
COM-style interface tied to LLVM 21 and CPU code generation. Reusing or extending that binary would
violate the pre-Blackwell typed-pointer contract. The new module therefore shares no LLVM object,
header, or library with `slang-compiler` or `slang-llvm`; only its versioned C structs and copied
bitcode bytes cross the boundary.

## Scope and Non-Goals

In scope:

- a standalone CMake project for one LLVM 14.0.6 module;
- a small internal versioned C ABI with version/build metadata, opaque handles, and caller-owned
  serialization buffers;
- general primitives sufficient to construct one caller-named, parameterless NVVM IR 2.0 kernel;
- host loading and copying the returned bitcode into a Slang artifact;
- coexistence with the LLVM 21 `slang-llvm` module in one process;
- real CUDA 12.2 libNVVM and `ptxas` validation on this machine; and
- binary export/import inspection on Windows.

Not in scope:

- general Slang IR traversal or lowering;
- scalar/pointer kernel parameters, control flow, arithmetic, address-space APIs, or libdevice;
- registration as a public downstream compiler or routing ordinary `-target ptx` requests;
- bundling LLVM 14 source/build products in the repository;
- defining the final cross-platform package/download URL or CI matrix;
- changing the existing LLVM 21 module; or
- performance conclusions from an empty kernel.

## Architecture and Invariants

The module ABI contains no C++ standard-library, COM, Slang IR, or LLVM types. Every structure has
an explicit size/version field or belongs to the versioned `_1` table. Names cross as a pointer plus
byte count and need not be NUL-terminated. LLVM modules, types, values, and blocks cross only as
opaque handles that remain owned by their module. Serialized assembly or bitcode is queried and
written with a two-call API into caller-owned storage.

The module constructs typed-pointer LLVM IR with target triple `nvptx64-nvidia-cuda`, the validated
64-bit NVPTX data layout, `!nvvmir.version = !{i32 2, i32 0}`, and a `kernel` entry in
`!nvvm.annotations`. It writes binary bitcode with LLVM 14's BitWriter. It does not parse textual IR
or embed/return the Slice 2 fixture.

LLVM 14 and LLVM 21 must be independently configured and linked. The LLVM 14 module uses static
component libraries and its own CRT-compatible allocation/release pair. On Windows, only the
versioned getter is exported and the DLL has no LLVM DLL import. On ELF/Mach-O, equivalent hidden
visibility/export-list rules must be represented in the standalone build even though validation is
local to Windows.

Module absence or ABI mismatch is an optional-component result, never a reason to change NVRTC
routing. Unsupported table size/version, null or cross-module live handles, invalid slices,
insufficient output capacity, or malformed module output must fail before libNVVM is called and
must not partially write output. As with other opaque C APIs, a forged or already-destroyed non-null
handle is outside the ABI contract; the header states that handles must be live and originate from
the module passed alongside them.

## Interfaces and Dependencies

Expected internal ABI header:

- `source/compiler-core/slang-nvvm-ir-builder-api.h`: C-compatible structs with fixed-width
  version/result/classification fields, native-size counts, opaque handles, calling/export macros,
  `SlangNVVMBuilderAPI_V1`, and the versioned symbol name/type. This header must compile as both C
  and C++ and include no LLVM headers.

Expected module:

- `source/slang-llvm-nvvm/CMakeLists.txt`: an independent project that requires exactly LLVM
  14.0.6 and links the smallest static component closure;
- `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`: LLVM construction, bitcode serialization, and the
  versioned function table; and
- optional platform export-list files if inspection shows they are needed.

Expected host/test integration:

- a small compiler-core loader/wrapper, or a test-local loader if no production consumer exists yet;
- `tools/slang-unit-test/unit-test-nvvm-builder.cpp`: ABI rejection, lifetime, runtime-name,
  LLVM-version, coexistence, real libNVVM, and `ptxas` coverage; and
- CMake/test dependency plumbing that is opt-in and does not make LLVM 14 part of the normal Slang
  configure.

Prototype dependencies live only under `build/nvvm-builder-deps/`. The initial exact dependency is
the official LLVM 14.0.6 source release, configured without projects, runtimes, targets, tools,
tests, examples, benchmarks, zlib, zstd, libxml2, libedit, RTTI, exceptions, or shared LLVM.

## Milestones

### Milestone 1: Establish the isolated LLVM 14 build

Hypothesis: LLVM Core, BitWriter, and Support can be built as a small static Windows dependency and
consumed through the LLVM build-tree CMake package without installing or altering the main Slang
configure.

Shallow-clone the official `llvmorg-14.0.6` tag into ignored
`build/nvvm-builder-deps/llvm-project/` and record its exact commit. Configure with Windows-native
`cmake.exe`, Visual Studio 2022 x64, Release libraries, static CRT, no target backends, and all
unrelated tools/projects/features disabled. Build only the component targets required by Core and
BitWriter. This follows Slang's historical acquisition method without reintroducing the old broad
Clang/target build.

Promotion criteria: the build tree provides `LLVMConfig.cmake`, LLVM 14 headers, and static
libraries usable by a separate consumer. Discard criteria: the minimal closure requires an
unbounded toolchain build, a dynamic LLVM dependency, incompatible runtime settings, or local
patches to upstream LLVM. Cleanup is removal of the exact ignored dependency root after its paths
and diagnostics are recorded.

### Milestone 2: Define and unit-test the ABI boundary

Add the C-compatible versioned header. Before using LLVM, build a fake in-process table in the unit
test to prove struct/version checks, pointer-plus-count handling, the size-query/write protocol,
insufficient-buffer behavior, invalid-output rejection, and exactly-once module destruction. Keep
the wrapper independent of how the module is discovered.

Promotion criteria: fake tests run in every build and no allocator, C++ class, or LLVM type crosses
the ABI. Discard criteria: the interface requires exposing LLVM objects or tying compiler-core to
LLVM 14 headers.

### Milestone 3: Build the LLVM 14 module

Implement the minimal general operations directly with LLVM 14 APIs and serialize assembly or
bitcode with LLVM printers/BitWriter. Configure the standalone project with `LLVM_DIR` pointing at
the isolated LLVM build tree. Copy only the resulting module into the main Debug binary directory
for local tests; do not copy LLVM libraries or headers.

Promotion criteria: the module reports LLVM 14.0.6/NVVM IR 2.0, emits nonempty `BC c0 de` bytes,
and its export/import table satisfies the isolation invariant. Discard criteria: LLVM symbols are
exported, an LLVM DLL is imported, the fixture is embedded, or the result uses opaque pointers.

### Milestone 4: Cross the real downstream boundary

Load both optional LLVM modules in one `slang-test` process. Generate `slice3Empty`, copy its bytes
to `ObjectCode + LLVMIR + Kernel`, compile with the discovered CUDA 12.2 libNVVM, assert PTX contains
`.visible .entry slice3Empty`, and assemble it for `sm_75` with `ptxas` using a unique temporary
path.

Promotion criteria: load/generate/verify/compile/assemble all succeed after LLVM 21 has been loaded,
and every opaque module handle is destroyed. A found module with incompatible ABI or failed
LLVM/NVVM operation is a failure, not a skip. Absence may skip only the environment-aware
real-module test.

### Milestone 5: Validate, audit, and distill

Build the affected main Debug targets and standalone Release module. Run the builder/NVVM prefixes,
LLVM downstream-version coverage, ordinary NVRTC regression, and `git diff --check`. Inspect the
module with Windows-native binary tools, check for generated PTX/cubin and dependency products
outside ignored `build/`, and perform the `AGENTS.md` new-helper/input-shape audit. Update the design
with evidence and leave the active plan uncommitted.

## Validation and Acceptance

Required local evidence:

- the standalone configure resolves exactly LLVM 14.0.6 and only static LLVM component libraries;
- the module exports the one documented versioned getter and imports no LLVM DLL;
- the C ABI header is LLVM-free and C-compatible;
- fake ABI tests cover version/size mismatch, byte-count handling, insufficient buffers, invalid
  serialization output, and module destruction;
- the real module reports LLVM 14.0.6 and NVVM IR 2.0;
- caller-selected `slice3Empty` appears in PTX, proving runtime IR construction;
- CUDA 12.2 libNVVM verifies/compiles the generated bitcode and `ptxas -arch=sm_75` accepts PTX;
- LLVM 21 and LLVM 14 modules work in the same test process;
- existing `slang-unit-test-tool/nvvm`, downstream version, and ordinary NVRTC tests remain green;
- normal builds do not require LLVM 14 or the new module; and
- no generated PTX/cubin or LLVM dependency is added to version control.

The exact commands used are recorded below. On this Windows checkout, all repository Git and CMake
operations use `git.exe` and `cmake.exe`.

## Reproduction Commands

The ignored local dependency and component libraries were created with:

```powershell
git.exe clone --depth 1 --branch llvmorg-14.0.6 `
    https://github.com/llvm/llvm-project.git `
    build/nvvm-builder-deps/llvm-project

cmake.exe -S build/nvvm-builder-deps/llvm-project/llvm `
    -B build/nvvm-builder-deps/llvm-build `
    -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded `
    -DBUILD_SHARED_LIBS=OFF `
    -DLLVM_BUILD_LLVM_C_DYLIB=OFF `
    -DLLVM_BUILD_LLVM_DYLIB=OFF `
    -DLLVM_LINK_LLVM_DYLIB=OFF `
    -DLLVM_INCLUDE_BENCHMARKS=OFF `
    -DLLVM_INCLUDE_DOCS=OFF `
    -DLLVM_INCLUDE_EXAMPLES=OFF `
    -DLLVM_INCLUDE_TESTS=OFF `
    -DLLVM_BUILD_TOOLS=OFF `
    -DLLVM_BUILD_UTILS=OFF `
    -DLLVM_ENABLE_BINDINGS=OFF `
    -DLLVM_ENABLE_PROJECTS= `
    -DLLVM_TARGETS_TO_BUILD= `
    -DLLVM_ENABLE_ZLIB=OFF `
    -DLLVM_ENABLE_ZSTD=OFF `
    -DLLVM_ENABLE_TERMINFO=OFF `
    -DLLVM_ENABLE_LIBXML2=OFF `
    -DLLVM_ENABLE_LIBEDIT=OFF `
    -DLLVM_ENABLE_RTTI=OFF `
    -DLLVM_ENABLE_EH=OFF `
    -DLLVM_ENABLE_ASSERTIONS=OFF `
    -DLLVM_ENABLE_DIA_SDK=OFF `
    -DLLVM_BUILD_RUNTIME=OFF `
    -DLLVM_USE_CRT_RELEASE=MT `
    -DLLVM_USE_CRT_DEBUG=MTd

cmake.exe --build build/nvvm-builder-deps/llvm-build `
    --config Release --target LLVMCore LLVMBitWriter LLVMSupport -- /m
```

The standalone module and focused host tests were configured, built, and run with:

```powershell
cmake.exe -S source/slang-llvm-nvvm `
    -B build/nvvm-builder-deps/slang-llvm-nvvm-build `
    -G "Visual Studio 17 2022" -A x64 `
    -DSLANG_NVVM_LLVM_DIR=C:/projects/slang/build/nvvm-builder-deps/llvm-build/lib/cmake/llvm `
    -DSLANG_SOURCE_DIR=C:/projects/slang

cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build `
    --config Release --target slang-llvm-nvvm -- /m
cmake.exe --build build --config Debug --target compiler-core -- /m
cmake.exe --build build --config Debug --target slang-unit-test `
    -- /p:BuildProjectReferences=false /m

$env:SLANG_NVVM_BUILDER_PATH = `
    (Resolve-Path -LiteralPath `
        'build/nvvm-builder-deps/slang-llvm-nvvm-build/Release').Path
build/Debug/bin/slang-test.exe slang-unit-test-tool/nvvm
build/Debug/bin/slang-test.exe slang-unit-test-tool/nvvmIRBuilderCoexistsWithLLVM21
```

The 25-test prefix passed. The separately invoked coexistence test also passed in a fresh process,
proving the LLVM-21-first load order without inheriting the unclosable LLVM 14 module from an
earlier builder test. With `SLANG_NVVM_BUILDER_PATH` unset, the two fake tests pass and the real
builder tests are reported as ignored. Setting it to a nonexistent directory makes a real builder
test fail with process exit code 1, proving that an explicitly configured broken module is not
silently skipped.

The final binary and nearby regression checks were:

```powershell
$nvvmDumpbin = `
    'C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/' + `
    '14.39.33519/bin/Hostx64/x64/dumpbin.exe'
& $nvvmDumpbin /exports `
    build/nvvm-builder-deps/slang-llvm-nvvm-build/Release/slang-llvm-nvvm.dll
& $nvvmDumpbin /dependents `
    build/nvvm-builder-deps/slang-llvm-nvvm-build/Release/slang-llvm-nvvm.dll

build/Debug/bin/slang-test.exe slang-unit-test-tool/unclosableSharedLibrary
build/Debug/bin/slang-test.exe slang-unit-test-tool/getDownstreamCompilerVersion
build/Debug/bin/slang-test.exe tests/cuda/sampler-comparison-state-unused
git.exe diff --check
```

## Failure and Recovery

All cloned source, LLVM build output, standalone module build output, and copied test DLLs are
derived artifacts under exact `build/` subdirectories. Rerunning configure/build is additive. Do not
delete or modify the user's unrelated untracked `external/slang-binaries/` directory.

If the LLVM dependency prototype fails, preserve only exact commands, versions, hashes, and
diagnostics in this plan and remove any source changes whose contract cannot be tested. If the
module builds but violates symbol isolation, do not load it alongside LLVM 21; fix the link/export
boundary first. If real libNVVM rejects its bitcode, retain the Slice 2 fixture path and compare
textual disassembly before changing either consumer or dialect rules.

The new module and loader remain optional and additive. Removing their source/tests or omitting the
module restores the Slice 2 state without affecting NVRTC or `slang-llvm`.

## Artifacts and Hand-Off

Retain as source changes only the versioned ABI, standalone module, host wrapper/test plumbing,
focused tests, and durable design updates. Keep this ExecPlan uncommitted. Keep LLVM source/build
trees, downloaded archives, copied DLLs, PTX, cubin, and inspection logs under ignored `build/` and
out of commits.

The eventual PR description must explain the concrete `slice3Empty` producer-to-consumer flow, the
module memory-ownership boundary, why LLVM 14 and LLVM 21 cannot share a configure or runtime symbol
namespace, which exact LLVM components are linked, and the import/export evidence proving the
isolation claim.

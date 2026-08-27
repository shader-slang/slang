# Link toolkit-matched libdevice and freeze floating-point policy

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the uncommitted
working log for Slice 18 of the direct NVVM backend experiment and must remain outside commits.

## Purpose and Observable Result

After this slice, `NVVMDownstreamCompiler` can compile an NVVM module that explicitly declares and
calls a libdevice function such as `__nv_sinf`. The request opts into the CUDA device library
through a new append-only `DownstreamCompileOptions` field. The compiler must load
`nvvm/libdevice/libdevice.10.bc` from the same selected CUDA toolkit root as libNVVM, add the user
module normally, add libdevice lazily when the API is present (or normally through the documented
compatibility fallback), verify and compile both with one exact option vector, return PTX accepted
by the matching `ptxas`, and execute representative sine inputs on the local GPU.

The slice also makes the existing single-precision policy an explicit tested matrix:

- default floating-point mode omits `-prec-div`, `-prec-sqrt`, and `-fma`;
- precise mode emits `-prec-div=1`, `-prec-sqrt=1`, and `-fma=0`;
- fast mode emits `-prec-div=0`, `-prec-sqrt=0`, and `-fma=1`;
- fp32 denormal Any omits `-ftz`, Preserve emits `-ftz=0`, and FlushToZero emits `-ftz=1`;
- unsupported non-default fp16/fp64 denormal policies and compiler-specific attempts to override
  the four managed option families fail before program creation.

This is a downstream compiler/toolkit policy slice. Direct linked-Slang-IR lowering of `float`,
`Ptr<float>`, `kIROp_Add` on float, and CUDA target-intrinsic helpers remains unsupported and
deterministically stops before provider discovery. No builder ABI operation is appended.

## Progress

- [x] (2026-08-27) Re-read `.agent/PLANS.md`, the Slice 17 plan and durable design, current
  downstream compiler/locator/options code, provider/emitter boundaries, and CUDA 12.9 installation.
- [x] (2026-08-27) Measured final linked IR for parameterized float arithmetic and `sin(float)`.
  Plain arithmetic is canonical float `mul`/`add`; `sin` remains a call to a CUDA target-intrinsic
  helper containing `GenericAsm("$P_sin($0)")`. Direct lowering stops at E52017
  `entry-point parameter` for plain f32 arithmetic and `helper function result type` for sine's
  float-returning call closure, both before provider discovery.
- [x] (2026-08-27) Confirmed CUDA 12.9 provides the selected toolkit's 486,144-byte
  `nvvm/libdevice/libdevice.10.bc`; explicit NVRTC produces f32 parameter/store PTX and inlines its
  sine implementation.
- [x] (2026-08-27) Chose an explicit append-only device-library demand signal rather than scanning
  LLVM bytes, always loading libdevice, or matching intrinsic/source spellings.
- [x] (2026-08-27) Added the version-safe compile-options field, selected-root propagation,
  libdevice identity, exact file loading, lazy/eager module addition, diagnostics, and
  floating-policy validation.
- [x] (2026-08-27) Extended fake coverage for byte/name/order preservation, selected-root coherency, failure
  cleanup, optional lazy-symbol fallback, option matrices, and unsupported/override policy.
- [x] (2026-08-27) Added real compiler-level libdevice/PTX, matching-toolkit `ptxas`, and GPU runtime evidence,
  while retaining direct-Slang float rejection.
- [x] (2026-08-27) Formatted, rebuilt outside the sandbox, passed focused NVVM 132/132 and the
  preservation matrix (1/1, 2/2, 1/1, 3/3, 2/2, 1/1), audited the diff and unchanged provider
  binary boundary, and updated durable evidence.
- [x] (2026-08-27) Removed Slice 18 probes, staged only the five intended tracked files, and
  committed them as `slice 18`.

## Surprises and Discoveries

- Observation: scalar `sin(float)` is not a canonical arithmetic opcode in final linked Slang IR.
  Evidence: `hlsl.meta.slang` selects CUDA `__intrinsic_asm "$P_sin($0)"`; the measured linked
  module contains a float helper whose body is `GenericAsm`, and the entry point calls it.
  Consequence: Slice 18 must not recognize `$P_sin`, helper names, or reconstructed CUDA syntax in
  the direct emitter. A later semantic intrinsic producer boundary is required.

- Observation: libNVVM does not automatically locate libdevice for a client.
  Evidence: NVIDIA's libdevice guide makes locating/reading the bitcode the client responsibility;
  the libNVVM API links modules added to one `nvvmProgram`. The current compiler adds only the user
  module and leaves `m_toolkitRoot` unused.
  Consequence: the downstream compiler owns coherent file selection and module addition.

- Observation: lazy addition is semantic selection, not the demand signal.
  Evidence: `nvvmLazyAddModuleToProgram` loads only symbols required by normally added modules, but
  a rootless compiler or incomplete toolkit still needs to compile libdevice-free integer modules.
  Consequence: add an explicit request field; do not infer demand by scanning text/bitcode and do
  not make every known-root compile depend on a readable libdevice file.

- Observation: locator-selected identity is lost today for injected loaders.
  Evidence: `_loadFromDirectories` returns only `ISlangSharedLibrary`; `init` can derive a path only
  from a real resolved symbol address, which points at the test executable for fake libraries.
  Consequence: carry the successful filesystem candidate path into compiler construction and use
  symbol introspection only as a fallback for logical/system loads.

- Observation: the four libNVVM controls are fp32 controls with documented defaults.
  Evidence: current NVIDIA libNVVM documentation defines `-ftz`, `-prec-div`, `-prec-sqrt`, and
  `-fma`; the existing Slang mapping already encodes the intended non-default modes but has only one
  combined positive test, ignores fp16/fp64 denormal requests, and lets later compiler-specific
  arguments contradict managed settings.
  Consequence: retain the mapping, freeze the full independent matrix, reject unsupported precisions
  and managed-option overrides, and pass the same final vector to verify and compile.

- Observation: pointer-aligning a terminal `bool` preserved the old size boundary but emitted MSVC
  C4324 padding warnings in every translation unit that included the options header.
  Evidence: the first Slice 18 build succeeded with the warning repeated across compiler-core and
  Slang; an x64 layout probe measured the prior size/field offset as 240 bytes and x86 class-layout
  reporting measured 148 bytes.
  Consequence: use naturally aligned pointer-sized 0/nonzero storage. The offset remains exactly
  240 on x64 and 148 on x86, the new x64 size is 248, and the warning disappears.

- Observation: the first focused run passed 129/132. The three failures were test-contract issues,
  not failed libdevice linking: a negative sine fixture expected the wrong direct rejection label,
  a lazy-add diagnostic searched for `libdevice` instead of the operation's `device-library`, and
  the real PTX classifier required optimizer-specific `sin.approx` text.
  Evidence: same-root `ptxas` and GPU sine execution already passed in that run. The independent
  review also found optional-prerequisite checks that could hide a selected-root regression,
  malformed enum fallthrough, CWD-sensitive relative candidate identity, and an `sm_70` runtime
  guard for `compute_75` PTX.
  Consequence: preflight optional toolkit components separately and require compile success;
  canonicalize filesystem candidates; reject malformed enums; check compute capability 7.5; prove
  self-containment by absence of unresolved `.extern .func` instead of optimizer spelling. The next
  complete run passed 131/132, with only the direct sine label remaining. Isolating that fixture
  proved the canonical helper-result boundary; restoring the whole table produced 132/132.

## Decision Log

- Decision: Slice 18 is the downstream libdevice/coherency and fp32 option-policy gate; direct f32
  emitter support is deferred.
  Rationale: this completes the named roadmap boundary without accepting non-semantic GenericAsm
  spellings or coupling library loading to a new LLVM-builder ABI. It is independently executable
  with a canonical handwritten NVVM module.
  Date/Author: 2026-08-27, Codex.

- Decision: append `uintptr_t requiresCUDADeviceLibrary = 0` to
  `DownstreamCompileOptions`.
  Rationale: an explicit semantic request avoids byte sniffing and preserves libdevice-free/rootless
  compiles. Natural pointer alignment starts the field after the previous structure size instead of
  reusing its tail padding, as required by the versioning contract, without creating a padded-field
  warning. Zero means false, any nonzero value means true, and an older-size caller defaults zero.
  Date/Author: 2026-08-27, Codex.

- Decision: derive the toolkit root from the exact successful library candidate when discovery
  knows it, falling back to the resolved libNVVM symbol path only for logical/system loading.
  Rationale: selection identity must survive fake/custom loaders, and libdevice must never be
  retried from a lower-precedence `CUDA_PATH`, `CUDA_HOME`, or PATH toolkit.
  Date/Author: 2026-08-27, Codex.

- Decision: normally add the user module first, then use `nvvmLazyAddModuleToProgram` for libdevice;
  if that optional symbol is absent, use ordinary `nvvmAddModuleToProgram` for libdevice.
  Rationale: NVIDIA documents lazy loading as the efficient form and ordinary addition as valid
  basic usage. The fallback preserves semantics on older compatible libraries and is explicitly
  tested rather than silently omitting the library.
  Date/Author: 2026-08-27, Codex.

- Decision: reject non-Any fp16/fp64 denormal modes and compiler-specific arguments in the managed
  `-ftz`, `-prec-div`, `-prec-sqrt`, or `-fma` families.
  Rationale: libNVVM's controls are specifically single precision, and policy must not depend on
  duplicate option ordering. The typed option fields remain the single source of truth.
  Date/Author: 2026-08-27, Codex.

- Decision: include the matched libdevice file timestamp in the downstream compiler version string
  when a coherent file is present.
  Rationale: replacing toolkit library bitcode can change PTX even when the libNVVM DLL timestamp is
  unchanged. Rootless compilers retain their existing libdevice-free identity.
  Date/Author: 2026-08-27, Codex.

## Outcomes and Retrospective

The x64 `DownstreamCompileOptions` extension begins at the exact prior 240-byte size and produces a
248-byte structure; the x86 prior size/field offset is 148 bytes. Older compatible prefixes copy no
request bytes and therefore retain zero demand. Fake tests prove rootless and known-root
demand-false compiles add only `slang-nvvm-input`; true demand reads the selected root's exact opaque
bytes (including an embedded NUL), adds user then `libdevice.10.bc`, uses lazy addition when present
and eager addition only when absent, does not retry a failed lazy add, and destroys every created
program once. The exact nine floating-mode/denormal combinations and malformed/unsupported/
override failures are covered before program creation with identical verify/compile vectors.

The selected toolkit was CUDA 12.9 at `C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.9`.
Its `nvvm\libdevice\libdevice.10.bc` is 486,144 bytes, has UTC timestamp 2025-05-27 09:50:51, and
SHA-256 `CD2824F8DD3F862B6B9259086F49F6CB56CA2547E14C61DE889C1C0D4A7DB175`; fake identity coverage
compares the exact selected file timestamp and proves rootless absence. Real compilation emitted a
self-contained `libdeviceSine` PTX entry with a global store and no unresolved `.extern .func`.
The same CUDA 12.9 root's `ptxas` accepted it. On the RTX 5090, inputs `0`, `0.5`, `-1.25`, and `20`
matched host `sinf` within `2e-6`.

The final Debug build succeeded without the earlier padding-warning flood. Pinned clang-format 17
reported no modifications. Focused NVVM passed 132/132. Preservation passed parser 1/1,
routing/hash 2/2, unsupported boundary 1/1, sampler 3/3, CUDA compile/pass-through 2/2, and runtime
dispatch 1/1. `dumpbin` still reports only `slang_getNVVMBuilderAPI_V1` and `_V2`; the provider has
only `KERNEL32.dll` plus delay-loaded `SHELL32.dll` and `ole32.dll`, with no LLVM DLL dependency.

Input-shape audit: the selected-path helper consumes only a discovery-proven filesystem candidate,
canonicalized at that boundary; it does not walk semantic graphs or retry an environment toolkit.
The exact libdevice-path helper maps only a retained coherent toolkit root. The eager branch is the
vendor-supported compatibility path for an absent optional symbol, never a retry after failure.
The request is caller-owned semantic state, not inferred from LLVM bytes, symbol spelling,
GenericAsm, or syntax reconstruction. Every new diagnostic rejects invalid input or reports a
specific external operation; no impossible shape silently defaults. Final commit: `slice 18`.

## Context and Current Pipeline

`NVVMDownstreamCompilerUtil::locateCompilers` searches explicit and automatic directories and
constructs one `NVVMDownstreamCompiler`. `NVVMDownstreamCompiler::init` resolves the required C API
and optional lazy-add/LLVM-version functions, records the library identity, and currently derives a
toolkit root only from the real function address. `compile` validates one LLVM IR kernel artifact,
computes options, creates a program, adds the user module, verifies, compiles, obtains PTX, and
destroys the program through `ScopeProgram`.

The direct Slang route constructs the same downstream options in `slang-emit.cpp`, but its finite IR
preflight currently accepts signed-i32 scalar/pointer shapes only. Because no accepted Slang IR can
request libdevice yet, this slice exercises the new signal through the compiler-level contract and
leaves the direct option false. A future principled semantic intrinsic lowering sets it when it emits
an actual libdevice declaration/call.

`DownstreamCompileOptions` is copied across component boundaries according to its leading
`VersionedStruct`. Additions must be terminal and must not occupy bytes that belonged to prior tail
padding. The new field therefore has pointer alignment and must have compile-time and runtime
offset/old-size coverage.

## Scope and Non-Goals

In scope:

- one explicit CUDA device-library demand field with version-compatible defaulting;
- exact selected-library-path/toolkit-root propagation for explicit filesystem discovery;
- toolkit-matched `nvvm/libdevice/libdevice.10.bc` loading and identity;
- normal user-module addition followed by lazy libdevice addition, with documented eager fallback;
- stable missing-root, missing/unreadable-file, module-add, verifier, and compiler diagnostics with
  exactly-once program destruction after creation;
- exact independent fp32 mode/denormal option matrices, duplicate-policy rejection, and fp16/fp64
  unsupported diagnostics before program creation;
- fake topology and real CUDA 12.9 `__nv_sinf` PTX/`ptxas`/runtime proof;
- existing integer, direct-routing, NVRTC, and runtime-dispatch preservation.

Out of scope:

- direct Slang f32 parameter, pointer, load/store, constant, arithmetic, phi, helper, or intrinsic
  lowering; any V2 builder ABI append;
- matching `$P_sin`, GenericAsm text, mangled names, or LLVM bytes to infer semantics/demand;
- automatic loading when the request field is false or retrying another toolkit root;
- half, double, bfloat16, fp8, vectors, matrices, complex numbers, or per-instruction fast-math;
- changing NVRTC's aggregate fast-math behavior or claiming route option parity;
- broad libdevice symbol-selection tables, sqrt/intrinsic producer redesign, performance claims,
  atomics, waves, resources, LTO, RDC, debug metadata, or packaging/CI rollout.

## Architecture and Invariants

The selected library candidate is the source of truth. A helper may derive a toolkit root only from
the canonical library path shape (`<root>/nvvm/bin[/x64]`, `<root>/nvvm/lib[64]`); it must return no
root for unrelated paths. Explicit-directory discovery passes the exact successful decorated
candidate path into `init`. Logical/system discovery passes no hint and uses symbol introspection as
the existing fallback. No environment search occurs during libdevice resolution.

When `requiresCUDADeviceLibrary` is false, compile must not require a toolkit root, read libdevice,
or call either library-add operation; all established integer behavior remains identical. Compiler
version identity is demand-independent and may stat a coherent retained libdevice path.
When true, option/source validation and libdevice reading finish before program creation. After
creation, the exact order is user normal-add, libdevice lazy-add or eager fallback, verify, compile.
Every post-creation failure destroys the program once and returns the usual associated diagnostic
artifact.

The libdevice bytes are opaque binary data and must be forwarded with their exact size, including
embedded NULs. The module name is `libdevice.10.bc`. Failure to find a coherent root or read that
exact path is an operational/request failure, not permission to search another toolkit or silently
compile without the requested library.

The managed option families are canonical. Mode and fp32-denormal controls compose independently;
the command-line order is deterministic but no duplicate family is accepted from
`compilerSpecificArguments`. `nvvmVerifyProgram` and `nvvmCompileProgram` receive element-for-element
identical vectors.

Every new helper/special case receives the repository's input-shape audit. Root derivation operates
only on a selected library file path supplied by discovery, not an arbitrary operand graph.
Libdevice addition operates only on an explicit semantic request, not content inspection. The eager
fallback is valid vendor API behavior, not a recovery search or alternate semantic representation.

## Plan of Work

First, append the pointer-sized request field and add static/runtime layout coverage proving that its
offset is at least the prior complete structure size and that a prior-size options object defaults
the field to false through `getCompatibleVersion`.

Second, factor one library-path-to-toolkit-root helper. Extend candidate loading to return the exact
successful decorated library path; pass it into compiler initialization. Preserve logical-name
loading and symbol-derived fallback. Centralize construction of the exact libdevice path.

Third, validate the floating policy before program creation. Reject unsupported fp16/fp64 modes and
managed compiler-specific duplicates, then emit the existing exact fp32 matrix. If libdevice is
requested, read its bytes before program creation and produce a targeted diagnostic on missing root
or read failure.

Fourth, add the user module normally and libdevice second. Dispatch to lazy-add when present and
ordinary add otherwise. Treat failures like the existing module-loading failure, including vendor
log retrieval and RAII cleanup. Add libdevice timestamp identity when a coherent file exists.

Fifth, extend the fake state and injected loader tests. Prove exact selected-root identity against a
conflicting environment root, exact bytes/name/order, no read/add without demand, lazy and eager
paths, missing-root/file and add failures, option matrices, duplicate/unsupported rejection, equal
verify/compile vectors, and prior-size defaulting.

Sixth, add a real compiler-level NVVM fixture declaring `float @__nv_sinf(float)`. Compile through an
explicit CUDA 12.9 root, classify the resolved/inlined PTX without freezing optimizer-dependent full
text, assemble it with the same root's `ptxas`, and run zero, finite positive/negative, and range-
reduction inputs with appropriate exact/tolerance comparisons. Keep the existing direct-Slang float
fixture as a negative boundary.

Finally, run pinned clang-format 17, rebuild the Release LLVM 14 provider if touched (it should not
be), rebuild the Debug host, run the full focused NVVM prefix and preservation matrix outside the
sandbox, inspect the diff and binary boundary, update design/ledger claims, remove Slice 18 probes,
stage only tracked intended files, and commit exactly `slice 18`.

## Concrete Steps and Validation

Run from `C:\src\slang` with Windows-native tools. All CMake builds and tests run outside the
sandbox as required by `AGENTS.md`.

    git.exe status --short
    cmake.exe --build build --config Debug --target slang-test -- /m
    build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm

Run the established preservation matrix:

    build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/parseCUDAEmissionMethods
    build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/cudaEmissionMethod
    build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/nvvm-unsupported-ir
    build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/sampler-comparison-state-unused
    build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/cuda-compile
    build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/coverageCudaRuntimeDispatch

Acceptance requires:

- compile-options old/new size and offset evidence with false default for the old prefix;
- no libdevice activity for false and an explicit pre-program error for requested rootless/missing
  library;
- exact selected-root sentinel bytes, user-then-library order, lazy and eager paths, stable module
  names, failure logs, and exactly-once cleanup;
- complete Default/Precise/Fast by Any/Preserve/FTZ matrix, no managed duplicates, no silent fp16/
  fp64 handling, and identical verifier/compiler vectors;
- real CUDA 12.9 `__nv_sinf` resolution to self-contained PTX, matching-root `ptxas` acceptance, and
  GPU runtime results for representative values;
- deterministic direct-Slang float rejection before builder discovery;
- focused and preservation matrices green after formatting;
- no builder export/dependency change and no unprincipled helper, fallback, syntax reconstruction,
  content sniffing, or secondary toolkit search.

## Idempotence and Recovery

All probes, builds, fake compiles, real compiles, `ptxas`, and runtime launches are safe to repeat.
Temporary toolkit roots must be test-owned and cleaned by existing RAII helpers. A failed libdevice
read occurs before program creation; a failed add/verify/compile returns an artifact and destroys the
program exactly once.

If the explicit successful candidate cannot be mapped to a toolkit root, stop and fix candidate
identity propagation; do not consult environment variables at compile time. If lazy addition fails,
surface that failure; eager addition is only the compatibility branch for an absent function, not a
retry after a semantic error. If real PTX retains an unresolved libdevice external, the link gate has
failed even if `ptxas` happens to accept relocatable syntax.

Do not delete/reset user work, stage `external/slang-binaries/`, this or other ExecPlans, or probe
artifacts. Remove only Slice 18 probes with `apply_patch` before committing. The default NVRTC route
and direct integer subset remain usable throughout.

## Artifacts and Hand-Off

Retain in this plan the final option layout, selected toolkit/libdevice identity, fake order/bytes/
failure evidence, real PTX/`ptxas`/runtime summaries, focused/preservation counts, formatter and
input-shape audits, and final commit hash. Distill stable architecture into
`docs/design/nvvm-backend.md`, durable results into
`docs/design/nvvm-backend-capability-ledger.md`, and the eventual five-part PR narrative. Keep this
plan untracked.

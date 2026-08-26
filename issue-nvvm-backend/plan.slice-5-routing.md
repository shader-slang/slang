# Select the CUDA emission method and route NVVM explicitly

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 5 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, callers can select how a PTX target is produced with one canonical target option:

```cpp
enum SlangEmitCUDAMethod
{
    SLANG_EMIT_CUDA_DEFAULT,
    SLANG_EMIT_CUDA_VIA_NVRTC,
    SLANG_EMIT_CUDA_VIA_NVVM,
};
```

The command-line spellings are `-emit-cuda-via-nvrtc` and `-emit-cuda-via-nvvm`. Default PTX
compilation remains on the established CUDA-source-to-NVRTC route, and the explicit
NVRTC spelling forces NVRTC rather than consulting a mutable transition override. Explicit NVVM
selection enters a dedicated PTX dispatch boundary and never falls back to NVRTC. Since lowering
Slang IR into valid NVVM IR is deliberately Slice 6, an ordinary Slang program selected for NVVM
ends at a stable diagnostic explaining that the lowering is not implemented yet.

The positive end-to-end proof for this slice uses the valid builder-generated bitcode established
in Slice 4 and submits it through the NVVM compiler registered in the global Slang session. This
demonstrates that option resolution, registry discovery, and the libNVVM downstream boundary are
real without inventing a placeholder kernel or sending CUDA source to an LLVM-bitcode consumer.

## Progress

- [x] (2026-08-26 13:09Z) Re-read `.agent/PLANS.md`, the durable NVVM design, the Slice 4 hand-off,
  and the current option, PTX dispatch, transition-map, downstream-registry, cache-hash, and test
  paths.
- [x] (2026-08-26 13:09Z) Completed independent read-only audits of the public option surface and
  the producer-to-consumer PTX routing boundary.
- [x] (2026-08-26 13:09Z) Created this bounded Slice 5 ExecPlan before implementation.
- [x] (2026-08-26 14:03Z) Appended `SlangEmitCUDAMethod` and option values 158-160, added both CLI
  spellings and reconstruction, and proved canonical, last-option-wins, target-isolated parsing.
- [x] (2026-08-26 14:03Z) Made the effective CUDA method one source of truth for PTX dispatch and
  downstream compiler identity in cache hashing, including `linkWithOptions` overrides.
- [x] (2026-08-26 14:03Z) Preserved default and pass-through behavior, forced explicit NVRTC, and
  added the dedicated NVVM boundary with stable no-fallback diagnostics.
- [x] (2026-08-26 14:03Z) Proved valid Slice 4 bitcode compiles through the session-registered NVVM
  compiler and that both expected entry points survive the handoff.
- [x] (2026-08-26 14:44Z) Built and ran focused and regression tests outside the sandbox, verified
  changed C++ with pinned clang-format 17, passed `git diff --check`, completed the
  input-shape/self-review audit, and updated the durable design and capability ledger.

## Surprises and Discoveries

- Observation: the code-generation transition map is keyed only by source and destination
  `CodeGenTarget`, and it asserts that the two targets differ. It cannot describe the private
  distinction between NVVM-flavored LLVM bitcode and other LLVM IR.
  Evidence: `CodeGenTransitionMap` in `source/slang/slang-global-session.h` and the single
  `CUDASource -> PTX` registration in `source/slang/slang-global-session.cpp`.
  Consequence: retain that transition for NVRTC and dispatch explicit NVVM directly at the PTX
  target boundary. Do not add a fake public target or an ambiguous second map entry.

- Observation: `emitWithDownstreamForEntryPoints` intentionally gives explicit pass-through mode
  precedence and can derive a source artifact directly from the translation unit.
  Evidence: `source/slang/slang-code-gen.cpp`, in the pass-through branch before ordinary source
  emission.
  Consequence: CUDA emission-method selection applies only to non-pass-through Slang compilation;
  raw `-pass-through` requests keep their established semantics.

- Observation: `getDownstreamCompilerRequiredForTarget(PTX)` and `Linkage::buildHash` currently
  identify NVRTC without access to the target request's effective method option.
  Evidence: `source/slang/slang-pass-through.cpp` and `source/slang/slang-session.cpp`.
  Consequence: routing and cache identity must resolve the selected compiler from the same
  effective option set rather than duplicating a PTX-is-NVRTC assumption.

- Observation: the registered NVVM downstream compiler accepts exactly one LLVM-IR kernel artifact
  in assembly or bitcode form. CUDA source, generic Slang IR, and CPU-oriented `ShaderLLVMIR` are
  not valid alternate spellings of that representation.
  Evidence: `NVVMDownstreamCompiler::compile` and its artifact validation in
  `source/compiler-core`.
  Consequence: Slice 5 may route and diagnose, but it cannot claim successful ordinary Slang
  lowering. The positive handoff test starts with builder-produced NVVM bitcode.

- Observation: identical repeated target formats are coalesced by command-line parsing rather than
  retained as independent `TargetDesc` entries.
  Evidence: an initial parser test with three PTX outputs observed only one PTX target; using PTX,
  DXIL, and CUDA source produced three independently inspectable target option sets.
  Consequence: the isolation test uses distinct formats while exercising both CUDA-selector orders.

- Observation: CLI/API target entries initially populate `TargetRequest`, but the public
  `linkWithOptions` API can supply the same canonical `CompilerOptionEntry` at component scope.
  Evidence: `TargetProgram` deliberately combines component/link-time options with target options,
  giving the component override precedence for code generation.
  Consequence: `CompilerOptionSet::getEmitCUDAMethod` is the semantic accessor. PTX dispatch and
  `ComponentType::getEntryPointHash` both consume the same `TargetProgram` effective set; session
  descriptor hashing uses the target set because no component exists there.

- Observation: `slang-unit-test-tool` is a separate DLL and cannot link non-exported methods from
  `source/slang`, including `CompilerOptionSet::getDefault` and command-line reconstruction.
  Evidence: direct unit calls produced unresolved external symbols while production `slangc`
  linked successfully.
  Consequence: the focused resolver shares an inline method-to-compiler mapping, and the registry
  proof starts with public session APIs. No internal method is exported solely for a test. The
  parser unit proves canonical storage; reconstruction remains a localized mapping parallel to
  CPU/SPIR-V.

- Observation: diagnostic-only invocations that do not request an artifact can finish before
  backend emission.
  Evidence: the new Slang test did not observe E52014 until its command requested a PTX output with
  `-o`; the direct `slangc` probe then returned E52014 and failure deterministically.
  Consequence: the diagnostic test requests an output explicitly so it exercises PTX dispatch.

- Observation: the public global-session API can trigger NVVM discovery but does not expose the
  selected `IDownstreamCompiler` object.
  Evidence: `checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)` populates the session registry, while
  the internal cache owns the registered compiler.
  Consequence: the test triggers discovery through the public API, then inspects the test-visible
  session cache to compile through that exact registered object without widening the public ABI.

- Observation: the first final-review implementation read only `TargetRequest`, while
  `TargetProgram` explicitly gives a public `linkWithOptions` entry precedence during codegen.
  Evidence: a focused API test selected NVVM at link time but took the default NVRTC path, and its
  hash matched the explicit-NVRTC program.
  Consequence: move the method accessor to `CompilerOptionSet`; pass the exact effective
  `TargetProgram` option set to both dispatch and `Linkage::buildHash`. The corrected test reaches
  E52014 and produces distinct NVRTC/NVVM hashes.

- Observation: true pass-through mode is state on `EndToEndCompileRequest`, whereas the public
  shader-hash API belongs to `ComponentType` and has no pass-through state.
  Evidence: all `Linkage::buildHash` callers are session/component digest APIs; pass-through
  dispatch obtains its compiler from the legacy end-to-end request instead.
  Consequence: the component hash intentionally describes the ordinary target-program route. The
  mixed pass-through test proves dispatch precedence, not a nonexistent pass-through hash contract.

- Observation: components do not retain their `ISession` owner independently.
  Evidence: the first public-API routing fixture returned a component after dropping its local
  session and then blocked during `link`; retaining the session made both tests complete.
  Consequence: the test fixture returns the component while callers retain the matching session for
  the component's full lifetime. This is test ownership, not a production routing workaround.

## Decision Log

- Decision: add `SlangEmitCUDAMethod` with default, NVRTC, and NVVM values and append canonical and
  CLI-only `CompilerOptionName` values at the existing ABI tail.
  Rationale: target compiler options are the existing stable API for analogous CPU and SPIR-V
  method selection. Appending preserves the documented numeric ABI; a new public struct member or
  vtable method is unnecessary.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the option system cannot round-trip one canonical derived enum without retaining
  CLI aliases.

- Decision: CLI aliases write only the canonical `EmitCUDAMethod` option and use last-option-wins
  semantics.
  Rationale: the two flags are mutually exclusive spellings of one value. Keeping aliases in the
  option set would create multiple semantic sources and unstable cache identities.
  Date/Author: 2026-08-26, Codex.
  Revisit when: compatibility evidence requires the historical SPIR-V priority behavior.

- Decision: default selects the established transition-driven NVRTC route, explicit NVRTC forces
  `PassThroughMode::NVRTC`, and explicit NVVM uses a direct PTX dispatcher.
  Rationale: this preserves existing behavior while making both explicit choices truthful. The
  transition map cannot distinguish the private NVVM artifact dialect.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the transition model gains a typed private-artifact key that can express the same
  invariant without a public target.

- Decision: explicit pass-through takes precedence over CUDA emission-method selection.
  Rationale: pass-through input is already in the source language owned by the requested compiler;
  hijacking it into Slang-to-NVVM lowering would change an established command-line contract.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the CLI explicitly deprecates or diagnoses mixed pass-through/method selections.

- Decision: ordinary NVVM-selected Slang compilation emits a named lowering-not-implemented
  diagnostic without synthesizing a module, requiring successful discovery, or falling back.
  Rationale: the cache hash may opportunistically query the selected compiler's version, but
  provider presence cannot turn E52014 into success or a different codegen result. A fabricated
  empty kernel would falsely claim that the program was compiled. A separate positive registry
  test proves the real downstream handoff using valid Slice 4 bitcode.
  Date/Author: 2026-08-26, Codex.
  Revisit when: Slice 6 can always produce a valid NVVM artifact before discovery.

- Decision: include the resolved downstream compiler's prelude/version identity in the cache hash.
  Rationale: the method value separates NVRTC and NVVM keys, but cached output must also change
  when the selected compiler implementation changes. Routing and identity must share one resolver.
  Date/Author: 2026-08-26, Codex.
  Revisit when: downstream compiler identity becomes an explicit artifact dependency elsewhere.

- Decision: read the canonical method from the effective `CompilerOptionSet` and interpret absence
  as `SLANG_EMIT_CUDA_DEFAULT`.
  Rationale: target-option parsing/API loading and component `linkWithOptions` are both valid
  producers. `TargetProgram` already owns their canonical precedence, so routing and component
  hashing must consume that existing representation rather than reconstruct it. The accessor
  mirrors `CompilerOptionSet::getIntOption` for the one explicit default, release-asserts malformed
  value shapes, and avoids a new exported testing surface.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the compiler-option default accessor becomes inline/exported or target-option
  ownership changes globally.

- Decision: when a component hash is requested, pass the exact `TargetProgram` effective option set
  into `Linkage::buildHash`.
  Rationale: the hash API promises to include backend-affecting target settings. Hashing the target
  request while codegen uses a component override would identify a different compiler. Reusing the
  already-merged option set keeps the method value, prelude, and downstream version aligned without
  inventing another precedence rule.
  Date/Author: 2026-08-26, Codex.
  Revisit when: component option hashing is redesigned globally.

## Outcomes and Retrospective

The implementation freezes enum values 0-2 and option values 158-160. Both CLI selectors collapse
to one canonical target option, with the later selector winning. A canonical `linkWithOptions`
entry participates in the `TargetProgram` effective set. Default PTX follows the existing
session transition, explicit NVRTC forces NVRTC, explicit pass-through retains precedence, and
explicit NVVM reaches E52014 without fallback. E52015 covers invalid API-provided values.

The final Debug build and post-format validation passed. The focused option/routing units passed
5/5; the component-hash and session-digest regressions passed 6/6; and the CUDA diagnostic,
default/explicit-NVRTC, and raw-pass-through lanes passed 6/6. The established downstream-version
and shared-library regressions each passed 1/1. With `SLANG_NVVM_BUILDER_PATH` selecting the Release
LLVM 14 provider, the registered handoff passed 1/1 and the complete NVVM prefix passed 33/33.
Pinned clang-format 17 reported no changed-line or new-file differences, and `git diff --check`
passed.

No custom equivalence, IR graph walk, syntax reconstruction, fabricated artifact, consumer-side
repair, or silent fallback was introduced. Slice 6 must replace `emitNVVMForEntryPoints`' stable
diagnostic with the producer that links/optimizes Slang IR, legalizes the minimal compute subset,
serializes exact NVVM bitcode, and only then invokes the registered compiler.

## Context and Current Pipeline

Consider this command, which today always uses NVRTC:

```text
slangc kernel.slang -target ptx -entry main
```

`TargetProgram` constructs a `CodeGenContext`. `_emitEntryPoints` groups PTX with targets requiring
a downstream compiler and calls `emitWithDownstreamForEntryPoints`. That function chooses
`CUDASource` through `_getDefaultSourceForTarget`, emits CUDA C++, then asks the global session for
the registered `CUDASource -> PTX` transition. `Session::_initCodeGenTransitionMap` registers NVRTC
for that transition. The downstream compiler receives a CUDA source kernel artifact and returns a
PTX artifact; diagnostics are extracted before the result is checked.

The new option is produced by CLI parsing, API target entries loaded through `Linkage::addTarget`,
or a canonical component entry passed to `linkWithOptions`. `TargetProgram`'s effective option set
is the source of truth read by both PTX routing and component cache identity. The PTX arm in
`_emitEntryPoints` preserves the generic path for default and pass-through requests, provides an
explicit NVRTC override when requested, and calls a named NVVM emission boundary for explicit
NVVM. In this slice that boundary emits the intentional unsupported-lowering diagnostic.
Separately, the NVVM unit fixture builds valid bitcode with `NVVMIRBuilder`, triggers
`PassThroughMode::NVVM` discovery through the public global session, and submits the exact artifact
to the registered compiler.

`Linkage::buildHash` hashes the canonical option automatically through `CompilerOptionSet`, then
asks which downstream compiler contributes prelude and version identity. That latter lookup must
use the same effective-option CUDA-method resolver as code generation so the output cache describes
the compiler that would actually run.

## Scope and Non-Goals

In scope:

- stable public enum and append-only option values;
- two CLI spellings, canonical parse state, serialization, and last-option-wins behavior;
- one effective-option method resolver shared by PTX dispatch and cache identity;
- unchanged default NVRTC behavior, truthful explicit NVRTC selection, and pass-through precedence;
- a dedicated explicit-NVVM boundary with a stable no-fallback diagnostic;
- registered-session compilation of valid builder-generated NVVM bitcode; and
- durable design/test status and focused regression evidence.

Out of scope:

- traversing, legalizing, or lowering Slang IR into LLVM/NVVM IR;
- classifying CUDA optimization passes for an NVVM representation;
- synthesizing placeholder kernels or rebuilding source syntax from checked IR;
- reusing the CPU-oriented `ShaderLLVMIR` target or adding a public NVVM-IR target;
- replacing or adding a second `CUDASource -> PTX` transition;
- changing the libNVVM provider ABI, LLVM 14 builder ABI, discovery policy, or PTX semantics;
- runtime GPU execution, performance comparison, or code-quality tuning; and
- changing raw pass-through behavior.

## Architecture and Invariants

There is one semantic source of truth: the effective canonical `EmitCUDAMethod` option.
CLI selector entries are parser vocabulary only and never survive in the target option set. The
resolver handles every known enum value explicitly. An invalid or future integer is not interpreted
as NVRTC or NVVM; it produces no downstream selection and reaches a stable invalid-method
diagnostic if compilation is attempted.

Default PTX retains the established producer/consumer shape:

```text
Slang IR -> CUDA C++ kernel artifact -> registered NVRTC -> PTX artifact
```

Explicit NVVM reserves this shape:

```text
Slang IR -> [Slice 6 NVVM lowering] -> exact LLVM-IR kernel artifact -> registered NVVM -> PTX
```

The bracketed producer does not exist yet. Slice 5 stops there deliberately. The positive registry
test begins after the bracket using exact builder bitcode, so the consumer contract is exercised
without accepting an accidental alternative representation.

The dispatcher owns backend selection; the downstream compiler owns only artifacts of its declared
kind, payload, style, and target. The transition map continues to own default CUDA-source routing.
Pass-through remains an earlier, explicit source-shape contract. Diagnostics produced by a
downstream compiler are always extracted before checking its result, preserving the Slice 3b
contract.

### Input-shape and special-case audit

Every new helper or branch must be inventoried before completion:

- `CompilerOptionSet::getEmitCUDAMethod` survives. Its exact input is the existing effective
  option set assembled by `TargetProgram`; absence is the intentional default spelling. It
  preserves the established target-versus-component precedence and release-asserts a malformed
  value kind.
- `getDownstreamCompilerRequiredForPTXTarget` survives. Codegen, the effective-option cache-hash
  wrapper, and the focused routing test all call the same exhaustive mapping; invalid values return
  `None` and reach E52015 rather than a fallback.
- The PTX dispatcher survives because PTX now has two intentionally selected producer shapes. It
  chooses a producer before representation-specific code and does not repair malformed IR.
- The NVVM unsupported boundary is temporary but principled: its exact input is ordinary Slang IR,
  produced by the existing front end, for which no canonical LLVM/NVVM representation exists in
  Slice 5. The test that fails without it is the no-fallback diagnostic test. Slice 6 replaces the
  diagnostic with a producer-side lowerer rather than adding consumer-side recovery.
- The registered-bitcode helper/test accepts only the exact artifact shape already validated by
  `NVVMDownstreamCompiler`; it must not walk or reinterpret arbitrary artifact graphs.
- The optional compiler override on `emitWithDownstreamForEntryPoints` survives. Default/explicit
  NVRTC still produce CUDA source; the override selects their request-local consumer without
  mutating the session transition. Explicit NVVM never enters this function.
- The test-only session-cache read survives because the public API can perform discovery but cannot
  return the registered compiler. It follows one cached pointer and does not rediscover or rewrite
  an artifact graph.
- No fallback, custom equivalence, syntax reconstruction, hard-coded IR walk, or silent default was
  found in the final self-review.

## Interfaces and Dependencies

`include/slang.h` appends the public enum and numeric `CompilerOptionName` entries after the current
tail. Existing numeric values never move. API clients select the method through
`TargetDesc::compilerOptionEntries`.

`source/slang/slang-options.cpp` registers and parses both CLI flags. `CompilerOptionSet` retains
the canonical integer and `writeCommandLineArgs` emits the corresponding flag for explicit values;
default emits neither.

An internal resolver, placed with existing downstream selection rather than in the libNVVM
provider, maps an effective PTX method to NVRTC, NVVM, or no compiler. `CompilerOptionSet` exposes
the minimal method query needed by both codegen and cache identity.
`emitWithDownstreamForEntryPoints` accepts an optional explicit compiler override; absence keeps
the current transition lookup intact.

The provider contract remains exact: one assembly-or-bitcode LLVM-IR kernel artifact enters
`NVVMDownstreamCompiler`, and one PTX artifact plus structured diagnostics leaves it. The LLVM 14
builder and libNVVM are optional runtime/build dependencies for the positive real-provider test;
parser, routing, diagnostic, and default-NVRTC tests must not require them.

## Milestones

1. Append the public enum/options and implement canonical CLI parsing and serialization in
   `include/slang.h`, `source/slang/slang-options.cpp`, and
   `source/slang/slang-compiler-options.cpp`. Extend command-line parser unit tests to prove each
   spelling, both orders, canonical-only storage, and multiple-target isolation. Keep command-line
   reconstruction as the inverse localized mapping without exporting an internal method for tests.

2. Add the effective-option downstream resolver in `source/slang/slang-pass-through.{h,cpp}` (or
   the narrowest existing owner), and use it from `Linkage::buildHash`. Add focused tests for
   default, NVRTC, NVVM, and invalid values so routing and identity cannot drift.

3. Refactor the PTX arm in `source/slang/slang-code-gen.{h,cpp}` into explicit dispatch. Preserve
   pass-through and default routes, allow explicit NVRTC to override the transition lookup, and add
   the named explicit-NVVM boundary. Add stable diagnostics in `source/slang/slang-diagnostics.lua`
   for unimplemented lowering and invalid option state.

4. Extend `tools/slang-unit-test/unit-test-nvvm-compiler.cpp` to compile valid Slice 4 builder
   bitcode using the compiler returned by the global session for `PassThroughMode::NVVM`. Assert
   successful PTX and diagnostic behavior, not merely that a compiler can be located directly.

5. Run layered outside-sandbox validation, then self-review every new helper/branch against the
   input-shape audit. Update `docs/design/nvvm-backend.md` with the settled option ABI, exact Slice
   5 status, and Slice 6 boundary.

## Validation and Acceptance

All CMake builds and all tests run outside the sandbox as required by `AGENTS.md`. From
`C:\src\slang`, use Windows-native tools:

```text
cmake.exe --build --preset debug --target slang-unit-test slangc
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/parseCUDAEmissionMethods
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/cudaEmissionMethodSelectsDownstreamCompiler
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/cudaEmissionMethodLinkOptionsAffectRoutingAndHash
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/invalidCUDAEmissionMethodIsDiagnosed
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/parseCommandLineArgs
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/specializedComponentHash
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/replayContextRecordGetSessionDescDigestCall
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/replayContextGetSessionDescDigestPlayback
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/SlangcCoverageManifestOutput
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/SlangcSeparateDebugInfoOutput
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/nvvm-routing-not-implemented
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/sampler-comparison-state-unused
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/cuda-compile
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/getDownstreamCompilerVersion
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/unclosableSharedLibrary
set SLANG_NVVM_BUILDER_PATH=<Release provider directory>
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvmIRBuilderCompilesScalarBitcodeThroughRegistry
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
<clang-format-17> --dry-run --Werror <changed C++ lines>
git.exe diff --check
```

The 2026-08-26 post-format run passed the focused option/routing units 5/5, cache/session
regressions 6/6, CUDA lanes 6/6, downstream/shared-library regressions 2/2, registered handoff 1/1,
and the complete NVVM prefix 33/33. Acceptance requires:

- public numeric values are appended and parser output contains only the canonical method;
- each explicit canonical value has an inverse CLI spelling, both orders are last-wins, and target
  settings do not leak;
- default PTX continues to compile through the established transition without selecting NVVM;
- explicit NVRTC forces NVRTC and produces established PTX behavior;
- explicit NVVM never falls back and ordinary Slang input gets the named Slice 5 diagnostic;
- raw pass-through still follows the explicit pass-through compiler;
- valid builder bitcode compiles through the session-registered NVVM compiler when the optional
  provider/libNVVM environment is available, and remains an intentional skip when it is absent;
- selected downstream compiler identity follows the same resolver as codegen; and
- formatting, `git diff --check`, focused tests, and relevant NVRTC/NVVM regressions pass.

The existing machine may eagerly probe unavailable APIs, so focused `slang-test` invocations keep
the established `-skip-api-detection` switch. Real NVVM tests use the standalone provider and CUDA
toolkit configuration already proven by Slice 4; absence is not allowed to turn an explicitly
selected real-provider invocation into success.

## Failure and Recovery

Every edit is additive or localized and rerunnable. Parser failures can be isolated without a
provider. Routing failures can be diagnosed with the stable explicit-NVVM test and default-NVRTC
regression before running the real provider. Registry failures should distinguish compiler lookup,
builder availability, libNVVM compilation, and PTX extraction through their structured diagnostics.

If the experimental route must be disabled, remove the two CLI spellings and explicit NVVM PTX
branch while leaving the canonical enum value reserved; the unchanged default transition restores
the established route. Never renumber a published option. Do not delete or replace the user's
untracked `external/slang-binaries/` directory. Generated build products and temporary PTX remain
outside commits.

## Artifacts and Hand-Off

Retain this plan locally and keep it out of commits. Checked-in artifacts are implementation,
focused tests, `docs/design/nvvm-backend.md`, and
`docs/design/nvvm-backend-capability-ledger.md`. Do not retain generated PTX or cubins.

The final hand-off must record the numeric enum/option ABI, CLI semantics, dispatch and cache
identity trace, exact diagnostics, optional provider requirements, outside-sandbox commands and
results, and the helper/special-case inventory. The required five-part PR narrative will explain
why the dedicated dispatcher owns a valid selected representation rather than masking malformed
IR, and why Slice 6 must implement the missing producer before the diagnostic can be removed.

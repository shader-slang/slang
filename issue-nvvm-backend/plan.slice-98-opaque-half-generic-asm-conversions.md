# Map opaque Half conversion helpers

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM recognizes the CUDA prelude's two exact scalar opaque-Half conversion
helpers and maps them to the existing typed floating-width conversion operation. The existing
`tests/compute/half-opaque-convert.slang` fixture should pass direct CUDA runtime and PTX lanes
without exposing arbitrary GenericAsm or revising the builder ABI.

## Progress

- [x] (2026-08-29) Reproduced the first post-Slice-97 boundary and dumped final linked IR.
- [x] (2026-08-29) Identified both exact retained helper bodies and signatures:
  `Half(Float)` with `GenericAsm("__float2half")`, and `Float(Half)` with
  `GenericAsm("__half2float($0)")`.
- [x] (2026-08-29) Audited the semantic catalog and provider. The existing generic
  `FLOAT_CONVERT` descriptor already emits `fptrunc`/`fpext` for scalar and vector values.
- [x] (2026-08-29) Added the two exact GenericAsm/signature rows to the semantic source of truth and admitted Float16
  in semantic signature matching.
- [x] (2026-08-29) Extended fake catalog dispatch and focused compiler coverage without adding a callback or
  parsing placeholders.
- [x] (2026-08-29) Registered direct runtime/PTX lanes for the existing opaque-conversion shader and validated PTX
  assembly.
- [x] (2026-08-29) Formatted, built, ran focused/full/changed-shader validation, self-reviewed, and
  updated the durable docs in preparation for the completed slice commit.

## Surprises and Discoveries

- The forward helper's canonical assembly string is `__float2half` without `$0`, but its owning
  function still has exactly one Float parameter. GenericAsm carries target-selected spelling;
  semantic arguments come from the verified helper signature, as they do for existing wave rows.
- The reverse helper is spelled `__half2float($0)`. Exact assembly alone is therefore neither
  symmetric nor sufficient; the existing catalog matcher already combines spelling with the full
  result/parameter signature.
- Float16 values and bidirectional Float16/Float32 conversion are already established by Slice 94
  at every provider layer. The remaining rejection is only that scalar GenericAsm signature
  matching hardcodes Float32 and the catalog has no rows for these producer spellings.
- Adding exact catalog rows also makes ordinary scalar FloatCast descriptors find those rows before
  the parameterized-family fallback. Both rows therefore retain the established
  `floating-point width conversion` diagnostic name. An initially narrower opaque-helper name
  broke the exact-capability diagnostic test even though lowering remained correct.
- The remaining Half texture fixtures all stop at texture-object helper parameters. The dumped
  `half-rw-texture-simple.slang` IR has exact Half/Half4 1D/2D texture helper signatures followed by
  `surf1Dread`, `surf2Dread`, and `surf2Dwrite` GenericAsm bodies. That resource/helper contract is
  the next independent slice boundary.

## Decision Log

- Decision: add two exact catalog rows using `SLANG_NVVM_VALUE_OP_FLOAT_CONVERT`.
  Rationale: the catalog is already the one source of truth from canonical GenericAsm spelling and
  exact signature to typed provider semantics. A helper-name matcher or a separate conversion table
  would duplicate that mapping.
  Date/author: 2026-08-29, Codex.
- Decision: generalize scalar floating semantic-type matching by exact established bit width.
  Rationale: the semantic descriptor already carries 16 or 32 bits and type lowering already
  recognizes both canonical scalar types. This removes the stale Float32-only gate without
  accepting Float64, BFloat16, FP8, or vectors in these scalar rows.
  Date/author: 2026-08-29, Codex.
- Decision: keep builder ABI revision 11 unchanged.
  Rationale: the provider's parameterized FloatConvert family already validates and emits the
  physical operation. GenericAsm text remains entirely inside compiler preflight.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

The existing shader calls:

    half ha = f32tof16_(a);
    float fa = f16tof32(ha);

After specialization, both prelude functions remain ordinary one-block helpers. Their sole
terminator is GenericAsm, and the entry calls them through the already-supported scalar helper ABI.
Direct preflight currently accepts the Half/Float signatures, then rejects the first terminator as
unsupported GenericAsm. No arithmetic, memory, resource, or ABI capability is missing.

The semantic builder already supports the exact descriptors
`Float16 <- Float32` and `Float32 <- Float16`; the LLVM provider produces `fptrunc` and `fpext`.
This slice should connect the canonical producer spelling to those descriptors and reuse the
existing call/return path.

## Scope and Non-Goals

In scope:

- exact `Func(Half, Float)` plus `GenericAsm("__float2half")`;
- exact `Func(Float, Half)` plus `GenericAsm("__half2float($0)")`;
- catalog-driven preflight, fake recording, and existing provider FloatConvert emission;
- direct runtime/PTX lanes for `half-opaque-convert.slang`.

Out of scope:

- arbitrary GenericAsm, placeholder parsing, helper-name matching, textual assembly transport, or
  other CUDA conversion spellings/rounding modes;
- vector GenericAsm conversions, Float64, BFloat16, FP8, bit reinterpretation, or saturating
  conversion;
- eliminating or rewriting the helpers upstream;
- any builder callback, operation ID, ABI revision, or LLVM serializer patch.

## Architecture and Invariants

- GenericAsm recognition requires exact text, a sole one-block helper terminator, exact result and
  parameter count/types, and the existing selected direct-call closure.
- Function parameters, not parsed `$0` tokens, are the semantic operands. The catalog row proves
  the expected count and types before emission collects them in declaration order.
- The compiler passes only a typed FloatConvert descriptor across the LLVM shield. Neither helper
  name nor GenericAsm text reaches the facade/provider.
- Ordinary FloatCast IR and these helpers use the same operation family, preserving one provider
  implementation and one width-validation policy.
- Mismatched text, result width, parameter width/count, extra blocks, or nonterminal GenericAsm
  remains deterministic before provider mutation.

## Interfaces and Dependencies

Append the two exact rows to `slang-nvvm-semantic-catalog.h`. Generalize
`_isNVVMSemanticType` in `slang-emit-nvvm.cpp` for exact selected scalar floating widths. Route the
new catalog conversion rows through the fake builder's existing typed parameterized-operation
recorder. Add focused source/negative cases to the existing NVVM emitter tests and direct test
directives to the existing shader.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds and tests run outside the sandbox per repository instructions.

## Milestones

1. Add the exact typed catalog rows and scalar Float16 signature matching.
2. Prove fake-boundary operation descriptors and exact rejection of adjacent GenericAsm shapes.
3. Register and run the existing shader through direct runtime and PTX, then assemble standalone
   PTX with CUDA 12.9.
4. Run complete NVVM regression validation, perform the helper/special-case audit, update this plan
   and the durable design ledger, and commit.

## Validation and Acceptance

Acceptance requires focused catalog/compiler and malformed-shape coverage; the complete
`slang-unit-test-tool/nvvm` prefix; every enabled `half-opaque-convert.slang` lane including new
direct runtime/PTX paths; standalone direct PTX; CUDA 12.9 PTX assembly; pinned clang-format 17;
and `git diff --check`.

Completed evidence:

- `nvvmSlangOpaqueHalfHelpersUseTypedFloatConversions` records exactly two typed scalar
  `FLOAT_CONVERT` operations: Float32 to Float16 and Float16 to Float32. It also records the two
  ordinary helper calls, one store, and one marked kernel.
- `nvvmSlangUnsupportedIRStopsBeforeEmission` passes with an exact `__float2half` helper that has
  an extra integer parameter. The complete signature mismatch remains E52017 `GenericAsm` before
  builder discovery; unrelated floating GenericAsm remains covered by the sine case.
- The standalone LLVM provider plus Release `slang-unit-test` and `slangc` targets build cleanly.
- The complete NVVM prefix passes 371/371 with the standalone provider.
- All four enabled `half-opaque-convert.slang` lanes pass: CUDA/NVRTC, direct CUDA runtime, LLVM,
  and direct PTX FileCheck. Two existing DX12/Vulkan lanes remain disabled.
- Standalone optimized direct output is 780 bytes of PTX. CUDA 12.9.86
  `ptxas -arch=sm_70` accepts it and emits a 2,792-byte cubin.

## Self-Review and Input-Shape Audit

Inventory the catalog rows, semantic-type relaxation, fake dispatch, and emitter test branches.
These GenericAsm bodies are canonical target-selected input from the CUDA prelude, so the direct
target owns mapping their exact semantic subset. Confirm that no helper name, loose substring,
placeholder parser, or arbitrary body is accepted. Removing either catalog row must reproduce the
measured GenericAsm failure in its direction; malformed signature/text cases must stop before
provider discovery.

The completed inventory is:

- The two catalog rows survive as the sole mapping from exact CUDA-selected spelling plus complete
  signature to the established typed operation. They contain no helper-name or placeholder logic.
- Exact selected-width matching in `_isNVVMSemanticType` survives because the semantic descriptor
  is already the canonical type source of truth. The lane-count gate keeps these rows scalar, and
  the existing selected-type classifier keeps Float64, BFloat16, and FP8 closed.
- The fake catalog dispatch branch survives solely to record catalog-resolved `FLOAT_CONVERT` with
  its complete descriptors, matching the provider's family-based implementation. It changes no
  production API.
- The malformed extra-parameter helper and existing unrelated-sine case prove that text alone does
  not admit an operation. No custom equivalence, syntax reconstruction, operand-graph walk,
  textual rewrite, or silent default was added.

## Failure and Recovery

If libNVVM rejects the already-proven scalar `fptrunc`/`fpext` when reached through these helpers,
record the exact IR and diagnostic and stop rather than preserving GenericAsm text or adding a
serializer rewrite. Generated dumps, PTX, and cubins stay under ignored `build/`. Never reset
unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record exact fake descriptors, helper IR, runtime output, PTX/cubin sizes, focused/full test counts,
next exact fixture stop, and self-review inventory. Distill the durable exact conversion mapping
into `docs/design/nvvm-backend.md`.

## Outcomes and Retrospective

The slice connected canonical prelude-produced representation to a provider capability that had
already been established, without widening the LLVM shield. Exact spelling and full semantic
signature validation make the two accepted helpers typed compiler semantics rather than general
inline assembly. Builder ABI revision 11 remains unchanged.

The durable capability ledger is the Slice 98 section in `docs/design/nvvm-backend.md`. Generated
IR dumps, PTX, and cubins remain under ignored `build/`. Slice 99 should start from the measured
texture-object helper-parameter boundary and treat texture handles plus their load/store semantics
as one coherent resource slice, rather than adding isolated GenericAsm strings.

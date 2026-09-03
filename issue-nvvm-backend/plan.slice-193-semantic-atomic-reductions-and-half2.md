# Preserve atomic-reduction semantics and support Half2 add

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The maintainers have
explicitly asked that each direct-NVVM slice commit include its plan, so this plan is a deliberate
exception to the repository's usual working-log policy.

## Purpose and Observable Result

Make the two remaining frozen-v1 workloads blocked by the canonical Half2 atomic-add reduction
compile and compare correctly through direct NVVM at O0 and O3. At the same time, remove CUDA text
matching from the complete ordinary atomic-reduction family by preserving the standard modules'
operation identity as semantic IR.

## Progress

- [x] (2026-09-03) Confirmed that two healthy frozen workloads share the exact Half2 add-reduction
  producer and that the existing typed atomic descriptor can represent its value shape.
- [x] (2026-09-03) Tagged the complete ordinary atomic-reduction producer family and removed its
  CUDA-text classifier.
- [x] (2026-09-03) Supported canonical global Float16x2 add through the existing typed atomic
  provider interface without revising ABI 35.
- [x] (2026-09-03) Added focused provider/compiler tests and promoted both unlocked frozen
  workloads.
- [x] (2026-09-03) Ran focused and broad regression gates, regenerated both corpus snapshots, and
  recorded output-quality evidence.
- [x] (2026-09-03) Completed the input-shape audit, documentation, report, and prepared the Slice
  193 commit.

## Surprises and Discoveries

- The provider atomic descriptor already carries a generic `SlangNVVMValueTypeDesc`; Half2 is an
  existing Float16 descriptor with lane count two. No callback or ABI-layout change is required.
- Direct NVVM currently recognizes all nine ordinary reduction spellings by exact CUDA assembly
  text even though both `core.meta.slang` and `hlsl.meta.slang` are canonical producers.
- The existing reduction path deliberately uses a value-returning atomic operation and discards
  the old value. Half2 can preserve that contract with PTX `atom.add.noftz.f16x2`; a separate void
  reduction provider API is not necessary for correctness.
- The discovery runner intentionally validates against the original immutable corpus-v1 selection
  artifact, not a current capability snapshot. Using `census.slice-146.tsv` preserves that contract
  while Slice 193's results remain a separate 452-row snapshot.

## Decision Log

- Decision: replace all ordinary reduction text matching in one slice, rather than adding a
  Half2-only spelling check. Each producer branch receives an operation-specific internal semantic
  ID. Date/author: 2026-09-03, Codex.
- Decision: widen the existing typed atomic operation family only for global Float16x2 add. Do not
  revise the provider ABI because its descriptor and callback already express the exact operation.
  Date/author: 2026-09-03, Codex.
- Decision: leave BFloat16/BFloat16x2 and non-add vector atomics unsupported. Their semantic types
  and target requirements are separate work and are not needed by either motivating workload.
  Date/author: 2026-09-03, Codex.

## Outcomes and Retrospective

Both frozen Half2 workloads now compare correctly in native CUDA and direct O0/O3. Frozen v1 keeps
452 identities and its 427 healthy denominator and advances from 421/421/421 to 423/423/423 with
two gains and no old-correct loss. Discovery keeps 82 identities and 72 healthy references at
72/72/72. The selected prefix passes 439/439 and the permanent category passes 102/102.

The refactor also removes nine CUDA spellings from direct-NVVM classification while preserving all
existing `Atomic<T>.reduce*` behavior. Focused O0/O3 PTX contains the packed Half2 atomic and
assembles for SM70, SM80, and SM90. Four healthy frozen gaps remain: three distinct substandard
helper-ABI types and one residual `RequirePrelude` marker.

## Context and Current Pipeline

Consider both frozen workloads:

```slang
__atomic_reduce_add(half2AddTarget[0], half2(0.125h, 0.25h));
__atomic_reduce_add(reinterpret<RWStructuredBuffer<half2>>(*inputBuffer)[0], half2(1.0h, 2.0h));
```

`hlsl.meta.slang::__atomic_reduce_add<T>` selects a CUDA `__intrinsic_asm` helper whose finalized
signature is `void(ref half2, half2, int)`. NVVM preflight currently reaches
`_resolveNVVMAtomicReduction`, compares the retained CUDA string, derives a typed atomic descriptor,
and rejects it because `NVVMSemantics::isSupported` admits only scalar floating add. Semantic
legalization already replaces tagged target assembly with `IRNVVMIntrinsic`, so the producer can
preserve operation identity without downstream text.

## Scope and Non-Goals

In scope are the nine ordinary atomic-reduction identities produced by the standard modules and
global relaxed Float16x2 add. Out of scope are BFloat16, other vector atomic operations, non-relaxed
ordering, shared Half2 reduction, changing scalar reductions from `atom` to `red`, user-authored
GenericAsm compatibility, and unrelated remaining frozen failures.

## Architecture and Invariants

- The standard-module branch owns reduction identity; direct NVVM never parses its CUDA spelling.
- The exact helper body remains one semantic terminator with a void result and either
  `(ref T, T, order)` or `(ref T, order)` parameters.
- Selected helper parameter types own value kind, bit width, and lane count. The semantic ID owns
  add/subtract/min/max/bitwise/increment/decrement behavior.
- Every call supplies a canonical global reference and a literal relaxed memory order before the
  helper is emitted.
- The typed atomic catalog is the single support policy shared by preflight, fake provider, and real
  provider. Float16x2 add is the only new admitted shape.

## Interfaces and Dependencies

Extend the compiler-internal semantic namespace introduced by Slice 192 and its source-name table.
Do not change `SlangNVVMAtomicOperationDesc`, the provider callback table, or ABI revision 35. The
LLVM 14 provider uses typed LLVM values at its boundary and one exact PTX inline-assembly operation
for the CUDA Half2 atomic instruction.

## Milestones

1. Add semantic IDs/names and annotate matching CUDA branches in `core.meta.slang` and
   `hlsl.meta.slang`.
2. Replace `_resolveNVVMAtomicReduction(IRGenericAsm, ...)` with an exact tagged-intrinsic resolver
   and route preflight, call validation, and emission through it.
3. Admit global Float16x2 add in the semantic catalog and teach the fake and LLVM 14 providers to
   validate and emit its typed operands/result.
4. Add provider serialization coverage and compiler fake-boundary coverage, then add permanent
   O0/O3 lanes to both motivating workloads.
5. Validate builds, focused/runtime/broad gates, frozen/discovery replays, PTX assembly and metrics;
   complete self-review, documentation, report, and commit.

## Validation and Acceptance

Run all builds and tests outside the sandbox with Windows-native tools. Acceptance requires:

- Release compiler/unit-test and isolated LLVM 14 provider builds pass.
- Focused real-provider tests prove the exact Half2 atomic shape and emitted PTX assembly contract.
- Compiler fake-provider tests prove atomic reduction selection comes from the semantic producer.
- Native CUDA and direct O0/O3 comparisons pass for both motivating workloads.
- The selected NVVM unit prefix and permanent NVVM category pass.
- Frozen v1 retains exactly 452 rows and 427 healthy references, gains both workloads in both modes,
  and has zero old-correct regressions. Discovery retains its separate denominator and metrics.
- Representative PTX assembles for SM70, SM80, and SM90 where the measurement harness permits.

## Failure and Recovery

If LLVM inline assembly does not verify, libNVVM rejects it, or `ptxas` rejects the generated PTX,
retain the semantic producer conversion but keep Float16x2 outside the supported atomic catalog and
record the exact provider diagnostic. Do not restore CUDA text matching or patch emitted text.
Generated probes, logs, and corpus mirrors stay below `build/` and may be regenerated safely.

## Artifacts and Hand-Off

Commit this completed plan with Slice 193 as explicitly requested. Retain permanent test directives,
exact frozen/discovery snapshots, a five-part report, and durable architecture/capability updates.
Keep transient IR, PTX, logs, and generated corpus mirrors below `build/`.

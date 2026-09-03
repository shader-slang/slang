# Carry zero-index bounds semantics into canonical IR

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The maintainers have
explicitly asked that each direct-NVVM slice commit include its plan, so this plan is a deliberate
exception to the repository's usual working-log policy.

## Purpose and Observable Result

Make `tests/compute/bound-check-zero-index.slang` produce the same deterministic result through
direct NVVM as through the established CUDA/NVRTC route at both O0 and O3. The source define
`SLANG_ENABLE_BOUND_ZERO_INDEX` currently affects only C++ emitted from Slang and expanded against
`slang-cuda-prelude.h`; direct NVVM consumes final linked IR that contains no corresponding
semantics. This slice must represent the option before target emission and lower it through a
canonical, reusable IR contract rather than recognizing this fixture or patching provider output.

## Progress

- [x] (2026-09-03) Reconfirmed Slice 188's census result: this is the only healthy frozen-v1 row
  that compiles and executes but differs in both direct modes.
- [x] (2026-09-03) Reconfirmed that CUDA's behavior comes from `SLANG_BOUND_CHECK`,
  `SLANG_BOUND_CHECK_BYTE_ADDRESS`, and `SLANG_BOUND_CHECK_FIXED_ARRAY` in the CUDA prelude.
- [x] (2026-09-03) Captured the exact final-IR access producers for all buffer and fixed-array
  accesses in the motivating kernel.
- [x] (2026-09-03) Implemented the finite direct-target legalization using ordinary typed IR
  comparisons, selection, resource dimensions, and arithmetic before preflight.
- [x] (2026-09-03) Added permanent direct O0/O3 result comparison; the rest of both corpora prove
  the inactive path because they do not select the option.
- [x] (2026-09-03) Built the final source, ran all required direct-NVVM gates, regenerated both
  corpora, measured the promoted workload, self-reviewed, and documented Slice 189.

## Surprises and Discoveries

- Slice 188 proved that passing or omitting `SLANG_ENABLE_BOUND_ZERO_INDEX` produces byte-identical
  direct O3 PTX, while the native CUDA PTX changes. The option is therefore lost before direct
  emission rather than miscompiled by libNVVM.
- Final linked IR already has one canonical form for each access: `ByteAddressBufferLoad`,
  `StructuredBufferLoad`, `RWStructuredBufferGetElementPtr`, and fixed-array `GetElement`. The
  option survives independently in the target option set, so a target legalization can combine
  those two existing sources of truth without adding another IR opcode.
- `ByteAddressBuffer.GetDimensions` itself obtains an equivalent `StructuredBuffer<uint>` view and
  multiplies its element count by four. The bounds legalization can reuse that same typed
  representation instead of exposing the provider's physical `{data, count}` resource layout.

## Decision Log

- Decision: compare observable result buffers, not textual PTX. Rationale: NVRTC and libNVVM are
  different compilation pipelines and are not expected to produce identical PTX. Date/author:
  2026-09-03, Codex.
- Decision: do not infer bounds policy from fixture names, source syntax, or downstream CUDA
  macros. The selected representation must be produced from an existing compile option and remain
  explicit until the target implements it. Date/author: 2026-09-03, Codex.
- Decision: materialize the policy in `legalizeIRForNVVM` immediately before capability preflight.
  Rationale: linked canonical access IR and the target option are both available there, while the
  generic provider operations already express every required comparison, select, dimension query,
  and arithmetic operation. Date/author: 2026-09-03, Codex.
- Decision: admit only the four access shapes observed in the fixture's final IR. Byte-address
  stores and explicit RW structured-buffer loads are plausible adjacent operations but are not
  proven by this slice and therefore remain unchanged. Date/author: 2026-09-03, Codex.

## Outcomes and Retrospective

The only healthy frozen runtime mismatch is now correct at O0 and O3. Frozen v1 advances from
418/418/418 to 419/419/419 over 427, with no old-correct regression and no all-row runtime
mismatch. Discovery remains 72/72/72 over 72. Existing generic operations were sufficient, so the
provider ABI remains revision 34. Keeping the transformation at the direct legalization boundary
also avoids adding another recipe family to the large emitter.

## Context and Current Pipeline

Consider the motivating accesses:

```slang
total += byteAddressBuffer.Load<int>(-tid * 4);
total += structuredBuffer[-tid];
total += fixedArray[-tid];
outputBuffer2[tid + 1] = total;
```

For generated CUDA, resource and array wrappers in `prelude/slang-cuda-prelude.h` rewrite an
out-of-range index to zero when `SLANG_ENABLE_BOUND_ZERO_INDEX` is defined. The final IR consumed
by `CodeGenContext::emitNVVMForEntryPoints` instead contains the already-lowered resource,
pointer, and array
operations. Slice 188 demonstrated that the command-line define does not change those operations.
This slice will trace the access lowering producers and carry an explicit semantic contract from
the earliest shared boundary that knows both the option and the access extent to the existing
direct-NVVM legalization/emission path.

## Scope and Non-Goals

In scope are the canonical access families exercised by the existing frozen test: byte-address
loads, structured-buffer reads and writes, and fixed-array indexing. Preserve behavior when the
option is absent and leave unproven adjacent access shapes outside this transformation. Out of
scope are
trap/clamp policies, texture surface boundary modes, global sanitizer design, arbitrary source
macro interpretation, and the unhealthy discovery matrix-layout mismatch.

## Architecture and Invariants

- Bounds policy is a compile semantic, not an emitter guess based on source text.
- Every rewritten index is evaluated once and selected as `index < count ? index : 0`; byte
  addresses use the CUDA-prelude contract `index <= sizeInBytes - elementSize ? index : 0` with
  the same unsigned arithmetic assumptions.
- The extent and element size come from canonical resource/array representation already retained
  in IR; they are not rediscovered from syntax.
- The transformation happens before direct-NVVM capability preflight so planning, SSA validation,
  and emission continue to consume ordinary supported operations.
- Builds without the option retain their current IR and behavior.

## Interfaces and Dependencies

Prefer an existing compiler option, IR decoration, or target legalization hook after tracing the
current pipeline. Add a new IR operation only if no existing canonical construct can carry the
policy through specialization and linking. Provider ABI revision 34 should remain unchanged:
comparison, select, arithmetic, resource dimensions, and aggregate operations already exist.

## Milestones

1. Dump and compare final linked IR with and without the define, identify each access producer,
   and locate the compilation request state that records preprocessor definitions.
2. Add a focused runtime contract proving the option produces bounds semantics and use the corpus
   to prove that omission leaves existing workloads unchanged.
3. Implement the producer/legalization change for the finite access family exercised by the test.
4. Promote the existing workload to permanent direct O0/O3 coverage once both result buffers match
   the healthy NVRTC reference.
5. Regenerate frozen-v1 and discovery artifacts, update the capability ledger/design narrative,
   and retain measurement evidence.

All five milestones are complete. The focused file passes 4/4, the selected prefix passes 437/437,
and the permanent category passes 94/94. The measurement gate assembled all five native/direct
SM70/SM80/SM90 configurations.

## Validation and Acceptance

Run all builds and tests outside the sandbox with Windows-native tools. At minimum:

- Build the Release provider and compiler/test targets.
- Run the focused `bound-check-zero-index` native, direct O0, and direct O3 lanes.
- Run focused unit coverage for any new compiler representation.
- Run the selected direct-NVVM regression prefix and permanent NVVM category.
- Regenerate the frozen and discovery census artifacts without changing either denominator.
- Confirm frozen v1 advances from 418 to 419 correct in O0, O3, and both, with zero old-correct
  regressions; discovery must remain 72/72/72.
- Assemble representative direct PTX for SM70, SM80, and SM90 where the harness supports it and
  record exploratory compile-time/PTX-size/runtime measurements against NVRTC.

## Failure and Recovery

All generated probes and corpus mirrors live below `build/` and may be safely regenerated. If the
option is not available as semantic compiler state, stop at the precise producer boundary and
record the missing interface rather than inspecting raw command strings in the emitter. If one
access family lacks an extent in canonical IR, retain a deterministic preflight failure for that
shape and do not broaden the transformation speculatively.

## Artifacts and Hand-Off

Commit the completed plan with Slice 189 as explicitly requested. Retain exact frozen-v1 and
discovery TSV/JSON snapshots, a five-part Slice 189 report, permanent test directives, and durable
architecture/capability-ledger updates. Keep transient IR/PTX/log probes below `build/`.

# Generalize the conventional CUDA global-parameter ABI

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM accepts a conventional CUDA global-parameter block with multiple
selected-scalar `RWStructuredBuffer` fields. It also preserves unused `SamplerState` or
`SamplerComparisonState` placeholders and their unsized arrays as opaque ABI storage without
claiming executable sampler semantics.

The existing `tests/cuda/sampler-comparison-state-unused.slang` compiles through the direct route
to a 40-byte constant block and accesses its float output resource at byte offset 8, matching CUDA
source/NVRTC. A new two-resource comparison shader runs through both routes on the GPU.

## Progress

- [x] (2026-08-28) Captured the final linked IR, generated CUDA source, and NVRTC PTX for the
  sampler fixture.
- [x] (2026-08-28) Identified and repaired the PTX-vs-CUDA producer mismatch in unsized-array field
  ordering.
- [x] (2026-08-28) Generalized conventional-block recognition and key-to-field-index lookup for
  multiple supported fields.
- [x] (2026-08-28) Added storage-only sampler and unsized sampler-array lowering with no builder
  ABI revision.
- [x] (2026-08-28) Added fake-provider, adjacent negative, direct PTX, and two-resource runtime
  comparison coverage.
- [x] (2026-08-28) Formatted, completed the Release/provider validation ladder, updated durable
  records, performed the input-shape self-review, and prepared Slice 75 for commit.

## Surprises and Discoveries

- The final PTX linked IR originally ordered fields as sampler, unsized sampler array, then output
  resource. CUDA source ordered them as sampler, output resource, then unsized array. The
  collector's existing CUDA rule moved unsized arrays last only when the final target enum was
  `CUDASource`; direct PTX therefore bypassed a producer rule that is part of the CUDA ABI.
- The shared IR layout treated an unsized array as a trailing flexible array with indeterminate
  size. CUDA does not: `Array<T>` in `slang-cuda-prelude.h` is a pointer plus `size_t`, with size 16
  and alignment 8. The AST CUDA layout already models this representation.
- `SamplerComparisonState` is intentionally a pointer-sized no-op placeholder in CUDA source. It
  must occupy storage so following fields retain ABI offsets, but no sampler operation should be
  admitted merely because the field exists.
- Multiple conventional resource fields require mapping the field-address key to its actual
  collected struct index. Retaining one selected field/index in the block descriptor encoded the
  initial proof-of-concept boundary rather than an ABI invariant.

## Decision Log

- Decision: apply the existing unsized-array-last collector rule to every target classified by
  `isCUDATarget`, including PTX.
  Rationale: field order is produced before either CUDA source emission or direct NVVM lowering;
  both consumers must receive one canonical CUDA ABI shape.
  Date/author: 2026-08-28, Codex.
- Decision: model CUDA unsized arrays as fixed pointer-plus-count storage in the shared IR CUDA
  layout rules.
  Rationale: this mirrors the CUDA prelude and the existing AST CUDA rule. An NVVM-emitter-local
  size exception would leave other IR layout consumers inconsistent.
  Date/author: 2026-08-28, Codex.
- Decision: introduce a storage role in NVVM type lowering and keep sampler placeholders illegal
  as executable values.
  Rationale: ABI preservation and runtime sampler semantics are different capabilities. The
  provider needs only generic i64, pointer, and struct construction for the former.
  Date/author: 2026-08-28, Codex.
- Decision: recognize every field structurally, then resolve executable field addresses by their
  semantic key.
  Rationale: field order is producer-owned and may differ from source declaration order. Key-based
  lookup is stable for multiple resources and does not encode positional knowledge.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The sampler fixture now takes the direct route. Its provider storage is exactly `{ i64,
{ float addrspace(1)*, i64 }, { i64 addrspace(1)*, i64 } }`, the resource field address uses index
1, and real PTX exposes a 40-byte constant symbol, an offset-8 `ld.const.u64`, and one global
32-bit store. All four registered source/PTX/direct lanes pass, and CUDA 12.9 `ptxas` accepts the
direct module for `sm_70`.

The former two-resource negative is now a positive CUDA/NVVM comparison named
`nvvm-conventional-global-multi-resource.slang`; both routes produce `11, 21, 31, 41` on the GPU.
A fixed sampler array preserves the adjacent E52017 boundary before builder discovery. The full
Release build and isolated provider build pass, and the complete NVVM prefix passes 337/337.
Builder ABI revision 3 is unchanged.

Self-review inventory: the `isCUDATarget` collector condition survives as a producer-side repair;
without it PTX and CUDA source build different physical structs from the same CUDA layout. The
unsized-array branch in `CUDALayoutRules` survives because both the prelude and AST CUDA rule define
the value as pointer plus count, and direct PTX confirms the resulting 40-byte ABI. The three
storage classifiers and `NVVMTypeUse::Storage` survive because they separate layout legality from
executable value legality; fixed arrays remain rejected. `_findNVVMConventionalGlobalField`
survives because `IRStructKey` is the canonical field identity and the collector owns position;
the helper does not search arbitrary graphs or reconstruct syntax. The fake provider's multi-field
acceptance survives as test-double bookkeeping for those structural types.

No fallback returns a default for malformed IR, no semantic value is reconstructed, and no
emitter-local byte offset compensates for a producer error. Reverting the collector condition
recreates the measured wrong field order; reverting the storage role recreates E52017 on the
existing sampler fixture; and the fixed-array negative demonstrates that passing tests did not
turn the conventional block into a general aggregate escape hatch.

## Context and Current Pipeline

Consider this source:

```slang
SamplerComparisonState g_scmp;
SamplerComparisonState g_scmpArray[];
RWStructuredBuffer<float> g_out;

[numthreads(1, 1, 1)]
void computeMain() { g_out[0] = 1.0f; }
```

CUDA source emits `GlobalParams` as sampler, output resource, then `Array<SamplerComparisonState>`.
Those fields occupy 8, 16, and 16 bytes respectively, all aligned to 8, for a 40-byte symbol. The
kernel loads the output data pointer from byte offset 8.

`collectGlobalUniformParameters` creates the synthesized constant-buffer struct and rewrites the
resource use to `get_field_addr` plus `load`. Direct preflight recognizes that compiler-produced
block. `NVVMTypeLoweringContext` lowers the storage struct through generic provider construction,
and emission resolves the resource key to the collected field index before calling
`emitStructFieldPointer`.

## Scope and Non-Goals

In scope are multiple selected-scalar read-write structured-buffer fields, storage-only sampler
placeholders, unsized arrays of sampler placeholders, CUDA field ordering, shared CUDA unsized-array
layout, key-based field addressing, the existing sampler fixture, and a two-resource runtime
comparison.

Out of scope are sampler or texture operations, fixed sampler arrays, ordinary scalar global
uniforms, read-only resources, texture storage, nested structs, general arrays, dynamic struct
indices, arbitrary global variables, and changes to the builder or provider ABI.

## Architecture and Invariants

The collector is the source of truth for physical field order. All CUDA-family targets move
unsized arrays after fixed-size fields. The shared CUDA layout is the source of truth for each
field's storage size and alignment.

The conventional block recognizer accepts only a nonempty synthesized parameter-group struct whose
fields are selected-scalar raw read-write structured buffers, sampler placeholders, or unsized
arrays of sampler placeholders. A field address is executable only when its key names an actual
field, the pointer pointee exactly matches that field type, and that type is a supported structured
buffer. Thus storage-only fields cannot enter load or sampler operation emission.

The provider interface remains structural. A sampler placeholder lowers to an opaque i64 slot; an
unsized sampler array lowers to `{ i64 addrspace(1)*, i64 }`; and a resource view retains
`{ element addrspace(1)*, i64 }`. None of these choices adds a semantic sampler callback or feature
flag.

## Interfaces and Dependencies

Change the global collector, shared IR CUDA layout rule, direct NVVM type lowering/emitter, fake
provider, CUDA tests, and durable design records. Reuse `isCUDATarget`, `IRSamplerStateTypeBase`,
`IRUnsizedArrayType`, and existing generic builder type/field operations.

Builder ABI revision 3, the real LLVM provider API, libNVVM API, and public Slang API are unchanged.

## Milestones

1. Make the collector produce the same CUDA physical field order for CUDASource and PTX, and make
   shared IR CUDA layout match `Array<T>` pointer-plus-count storage.
2. Replace the one-field descriptor with structural block recognition plus key-based field lookup.
3. Add storage-role lowering for sampler placeholders and unsized sampler arrays while keeping
   executable value roles closed.
4. Add a fake sampler/resource/array graph, a fixed-sampler-array negative, a direct lane on the
   existing fixture, and a two-resource GPU comparison.
5. Format, validate Release and standalone provider builds, run the full NVVM prefix, assemble
   real PTX with `ptxas`, update durable records, self-review, and commit.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- fake emission observes three ABI-ordered storage fields and accesses the resource at index 1;
- a fixed sampler array stops with E52017 before provider discovery;
- `sampler-comparison-state-unused.slang` passes all CUDA/CUDA-PTX/direct lanes and direct PTX has a
  40-byte constant symbol, an offset-8 resource load, and a global float store;
- the two-resource comparison returns `11, 21, 31, 41` through both CUDA/NVRTC and direct libNVVM;
- real direct PTX is accepted by CUDA 12.9 `ptxas` for `sm_70`;
- Release host and standalone provider builds pass, as does the complete NVVM test prefix;
- formatting and `git diff --check` pass; and
- `external/slang-binaries/` and generated build artifacts remain unstaged.

## Failure and Recovery

If field offsets diverge, inspect the collected struct order and generated CUDA source before
changing NVVM emission. Do not compensate with hard-coded byte offsets or source declaration
indices. If a storage-only sampler becomes executable, tighten preflight/type-use classification
instead of adding dummy sampler behavior.

All implementation changes are isolated to CUDA field collection/layout and the experimental
direct route. Removing the direct storage classifiers restores the measured E52017 boundary; the
collector/layout repairs should remain because they align existing CUDA-family producers.

## Artifacts and Hand-Off

Keep diagnostic IR, generated CUDA, PTX, and `ptxas` outputs under ignored `build/` paths. Distill
the accepted field family, storage/value distinction, ABI ordering, validation evidence, and next
corpus stop into `docs/design/nvvm-backend.md` and the capability ledger. Complete the outcome and
self-review sections before committing this plan with Slice 75.

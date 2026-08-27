# Slice 26: Store signed i32 through a raw CUDA RWStructuredBuffer

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user requires each
completed slice plan to ship with its implementation, so this plan will be committed with Slice 26.

## Purpose and Observable Result

After this slice, the direct NVVM route accepts the smallest raw CUDA resource kernel:

```slang
[CUDAKernel]
void computeMain(RWStructuredBuffer<int> destination, uniform int index)
{
    destination[index] = 42;
}
```

The implementation must follow the exact final-linked resource representation. Direct NVVM and
NVRTC must agree on the 16-byte `{device pointer, element count}` launch value, device-global
signed-i32 storage, `ptxas` acceptance, and one-element runtime output. This slice must not infer
support for textures, samplers, arbitrary structured-buffer element types, thread builtins, or
conventional global-parameter lowering.

## Progress

- [x] (2026-08-27) Started from committed Slice 25 `3ebc44f22`, focused NVVM 180/180, preservation
  10/10, and a clean tracked worktree.
- [x] (2026-08-27) Measured final linked raw-resource IR, the pre-change E52017 entry-parameter
  boundary, and NVRTC's aligned 16-byte parameter / first-u64 device pointer / global store ABI.
- [x] (2026-08-27) Audited the exact raw producer/consumer boundary and kept conventional global
  parameter-block lowering as a distinct future ABI.
- [x] (2026-08-27) Appended one coherent provider type/addressing capability for the exact
  `{AS1 i32 pointer, i64 count}` aggregate and non-`inbounds` element address.
- [x] (2026-08-27) Added negotiation, invalid/no-mutation, fake topology, exact-Slice-25-provider,
  adjacent-negative, PTX, both-`ptxas`, and one-element GPU runtime evidence.
- [x] (2026-08-27) Applied pinned formatting; rebuilt Release provider/tests and Debug preservation
  targets outside the sandbox; passed focused 188/188 and preservation 10/10; updated durable
  docs/ledger; inspected the DLL; removed probes; and completed the self-review.

## Surprises and Discoveries

- Observation: a conventional global resource and a raw CUDA resource have distinct final ABIs.
  Evidence: the conventional kernel retains `ConstantBuffer<GlobalParams>`, `get_field_addr`, a
  load, and `rwstructuredBufferGetElementPtr`; NVRTC exposes a 16-byte `SLANG_globalParams`
  constant symbol. The raw kernel retains one exact `RWStructuredBuffer<int>` parameter and the
  element-pointer operation; NVRTC exposes one `.param .align 8 .b8[16]` kernel value.
  Consequence: Slice 26 owns only the raw resource value/addressing contract. Conventional global
  parameter-block storage is a later slice, not an alternate spelling accepted here.

- Observation: CUDA's `RWStructuredBuffer<int>` launch value is exactly the prelude representation.
  Evidence: `slang-cuda-prelude.h` defines `data` followed by `size_t count`; NVRTC loads the first
  u64 from the 16-byte parameter, converts it to a global pointer, and stores `42` through it.
  Consequence: model one exact LLVM aggregate `{i32 addrspace(1)*, i64}` and extract/address it
  through dedicated provider operations; do not flatten it to two kernel parameters or discard the
  count field from the ABI merely because this operation does not read it.

- Observation: the coherent append grows V2 from 368/204 bytes to 384/212 bytes on x64/x86.
  Evidence: strict-C and C++ probes agree; the exact Slice 25 prefix remains accepted, every partial
  size and either null complete callback are rejected, and future-larger tables are clamped.
  Consequence: type construction and element addressing are one all-or-none resource capability
  with a stable `raw-rw-structured-buffer-i32` identity bit.

- Observation: direct and NVRTC PTX preserve the same launch ABI despite very different module
  sizes.
  Evidence: the 611-byte direct output and 8,559-byte NVRTC output both declare aligned 16-byte
  `.b8` resource storage followed by i32 index, load the first u64 field, scale the signed index,
  and store globally. Both `ptxas` lanes use four registers, 372 bytes `cmem[0]`, no barriers,
  stack, or spills; RTX 5090 execution writes 42 through the exact raw argument on both routes.
  Consequence: classify semantic ABI/addressing/storage behavior rather than comparing PTX text or
  treating module size as a performance result.

- Observation: provider assembly cannot be verified while its current test block is unterminated,
  and the existing PTX signature classifier did not recognize `.b8` aggregate declarations.
  Evidence: the first full run failed only the new invalid-provider test's premature serialization
  and the new differential classifier; runtime and both assembler lanes passed. Exact final
  instruction counts prove invalid calls inserted nothing, and adding token-safe 8-bit declaration
  recognition lets the established parameter classifier represent `.b8[16]` without special-case
  parsing.
  Consequence: the corrected harness assertions pass individually and in the final 188/188 run;
  no production change was needed for either failure.

## Decision Log

- Decision: begin Bucket 6 with raw-CUDA signed-i32-indexed `RWStructuredBuffer<int>` storage.
  Rationale: it is a real resource value whose underlying payload can reuse the
  established device-i32 pointer, constant, addressing, and store capabilities. It avoids coupling
  the first resource slice to texture/sampler handles, intrinsic spelling, floating-point/vector
  support, dispatch builtins, conventional global parameter blocks, or runtime descriptor policy.
  Date/author: 2026-08-27, Codex.
  Revisit when: final linking does not preserve a bounded canonical resource-to-pointer shape.

- Decision: expose dedicated provider operations for the raw aggregate type and its element
  pointer instead of constructing or flattening this LLVM shape in the Slang host.
  Rationale: LLVM types and aggregate instructions belong to the isolated LLVM-owning DLL. One
  provider helper remains the structural source of truth, while Slang preflight owns the exact
  final-linked semantic type and producer relationship.
  Date/author: 2026-08-27, Codex.
  Revisit when: a later resource family proves a principled shared provider abstraction without
  weakening the exact Slice 26 boundary.

## Outcomes and Retrospective

Slice 26 accepts exactly the final-linked raw `[CUDAKernel]`
`RWStructuredBuffer<int, DefaultLayout>` parameter and its canonical
`kIROp_RWStructuredBufferGetElementPtr` producer. Slang preflight checks the exact resource,
signed-i32 index, canonical generic read-write scalar-layout result pointer, availability, and
producer relationship. The provider revalidates the exact `{ i32 addrspace(1)*, i64 }` aggregate,
index, module/function ownership, dominance, and insertion state before its only mutations: one
field-zero `extractvalue` and one non-`inbounds` GEP. The established signed-i32 constant/store
path consumes the result.

This is the correct consumer boundary. The final producer is intentional and stable; no custom
equivalence, syntax reconstruction, arbitrary operand walk, resource flattening, fallback, or
producer-side repair survives the diff audit. Conventional globals measurably retain
`ConstantBuffer<GlobalParams>`, `get_field_addr`, and load before resource addressing, so accepting
that shape here would hide a separate ABI rather than canonicalize an accidental spelling.

The coherent private V2 suffix is 384 bytes on x64 and 212 bytes on x86. Exact 368/204-byte Slice
25 providers retain every established program and gate this resource before module creation;
partial sizes and either null callback are rejected. Adjacent conventional, read-only, unsigned,
and floating resource shapes, plus raw read-write resource loads and atomics, stop at E52017 before
provider discovery. Invalid provider calls clear outputs; exact final assembly counts prove that
rejected calls inserted no resource-address instructions.

Direct NVVM and NVRTC agree on the aligned 16-byte raw resource plus i32 index launch ABI,
first-u64 data-pointer field, signed index scaling, and global u32 store. CUDA 12.9 `ptxas` accepts
both; RTX 5090 execution stores 42 through the exact one-element `{device pointer, count}` value.
The Release focused suite passes 188/188 and the Debug preservation matrix passes 10/10. The
Release provider exports only `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2`, depends ordinarily on `KERNEL32.dll` with delayed `SHELL32.dll` and
`ole32.dll`, and has no process-visible LLVM DLL dependency.

## Context and Current Pipeline

Through Slice 25, direct NVVM accepts raw `[CUDAKernel]` signed-i32 and device-pointer parameters,
scalar addressing/storage, fixed signed-i32 arrays, direct calls, control flow/SSA, selected scalar
operations, and one relaxed global atomic. Resource parameters remain rejected. CUDA source
emission deliberately keeps resource legalization disabled and relies on CUDA prelude types; the
direct backend must define its own concrete resource representation from final linked IR.

The motivating resource's element and operation are deliberately already supported: signed `i32`,
constant zero, and device-global store. The first milestone determines whether the missing boundary
is raw resource parameter mapping, a canonical resource-to-pointer instruction, or a
larger aggregate/template representation. Only the measured canonical form may be admitted.

## Scope and Non-Goals

In scope is one raw-CUDA writable structured-buffer parameter of scalar signed `i32`, one signed
`i32` element index, one store, one compute entry point, append-only negotiation, deterministic
adjacent negatives, direct/NVRTC PTX, `ptxas`, and GPU runtime output.

Out of scope are conventional global resources and their `SLANG_globalParams` symbol, read-only
buffers, append/consume buffers, byte-address buffers, uniform/constant buffers, arrays of
resources, non-i32 or aggregate elements, textures, samplers, surfaces, bindless/descriptor heaps,
additional address spaces, resource queries/atomics, dispatch builtins, and optimization or
performance claims beyond measurements collected here.

## Architecture and Invariants

The final linked IR producer is the source of truth. Its distinct raw `RWStructuredBuffer<int>`
value is intentionally the same 16-byte aggregate as the CUDA prelude: device pointer then count.
Model that value explicitly in preflight and the append-only provider API; do not reinterpret it as
a raw pointer, flatten its ABI, or rebuild source syntax. The element-pointer provider operation
must validate exact aggregate type, index type, ownership, availability, and insertion state before
extracting the data pointer and applying ordinary non-`inbounds` addressing.

An exact Slice 25 provider must remain usable for all older programs. Any appended prefix must have
one coherent minimum, reject partial sizes and null complete callbacks, gate before module creation,
and participate in the provider identity. No fallback to NVRTC or serialized-text manipulation.

## Interfaces and Dependencies

Potential changes are limited to the private V2 provider ABI, its strict-C layout probes, the host
wrapper/identity, exact direct-NVVM preflight/emission, focused unit tests, and durable design/ledger
documents. The baseline remains LLVM 14.0.6 producing negotiated NVVM IR 2.0 text for CUDA 12.9
libNVVM, NVRTC, and `ptxas`, with RTX 5090 runtime evidence. No public Slang API change.

## Milestones

1. Dump all late IR stages, compile the raw source through pre-change direct NVVM and NVRTC,
   and record the exact first unsupported producer plus NVRTC launch ABI/PTX.
2. Trace that producer back through CUDA linking/legalization. Audit whether the shape is canonical
   and whether an upstream target distinction is required.
3. Implement the smallest exact mapping, reusing established pointer/type/store operations and
   appending provider capability only when the LLVM construction genuinely requires it.
4. Prove negotiation/no-mutation, exact final topology, older-provider behavior, and rejection of
   adjacent resource/element/access/index shapes before provider mutation.
5. Compare direct/NVRTC PTX semantically, assemble both with matching-root `ptxas`, and execute a
   one-element raw resource value through both routes.
6. Apply pinned formatting, rebuild/test outside the sandbox, inspect exports/dependencies, update
   docs and this plan, remove probes, perform the input-shape audit, and commit intended files only.

## Validation and Acceptance

Build the provider and Release test targets outside the sandbox, then run the focused
`slang-unit-test-tool/nvvm` prefix. Run the established preservation matrix: parser 1/1,
routing/hash 2/2, unsupported boundary 1/1, sampler 3/3, CUDA compile/pass-through 2/2, and runtime
dispatch 1/1. Acceptance additionally requires both `ptxas` lanes, direct/NVRTC runtime agreement,
the two-export DLL allowlist, no process-visible LLVM DLL, pinned formatting, `git diff --check`, and
a self-review inventory of every helper/fallback/special case.

## Failure and Recovery

Probes, builds, tests, formatting, and binary inspection are safe to repeat. If the final resource
shape is broad, target-specific source syntax, or requires unresolved runtime descriptor semantics,
record the evidence and stop before an unprincipled downstream representation. Do not delete or
stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep final IR, ABI sizes, provider assembly, PTX classification, assembler/runtime results, test
counts, binary surface, and input-shape audit in this plan. Distill durable architecture into
`docs/design/nvvm-backend.md` and coverage into the capability ledger. Remove probes before the
completed plan and implementation are committed with first commit line `slice 26`.

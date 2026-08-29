# Admit selected vector structured-buffer transport and swizzled stores

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM backend accepts `StructuredBuffer<T>` and
`RWStructuredBuffer<T>` when `T` is one of the already-supported two-, three-, or four-lane
32-bit integer or floating-point value vectors. Whole-vector resource loads and stores preserve
the existing CUDA vector layout, and canonical `swizzledStore` writes only the named destination
lanes. The existing `tests/compute/structured-buffer-load.slang` and
`tests/compute/structured-buffer-swizzle-store.slang` shaders must execute through direct libNVVM,
emit PTX with global-memory vector traffic, and assemble with CUDA 12.9 `ptxas -arch=sm_70`.

## Progress

- [x] (2026-08-29) Measured both existing shaders and identified vector resource element storage,
  followed by canonical `swizzledStore` for the second shader, as their first unsupported shapes.
- [x] (2026-08-29) Re-read the ExecPlan standard and audited the existing value-vector,
  raw-buffer, pointer, load/store, and fake-provider contracts.
- [x] (2026-08-29) Extended one resource-element classifier to the existing selected 32-bit
  numeric vector family and admitted matching read-only/read-write loads.
- [x] (2026-08-29) Validated and emitted canonical constant-lane vector swizzled stores without
  adding a builder ABI
  operation.
- [x] (2026-08-29) Added focused fake-provider coverage and registered both existing shaders for
  direct runtime, PTX, and assembler evidence.
- [x] (2026-08-29) Formatted, built, ran focused and CUDA-scoped validation, updated durable
  capability evidence, self-reviewed, and prepared the completed plan with the implementation for
  commit.

## Surprises and Discoveries

- The raw-buffer lowering, numeric value validator, vector type lowering, alignment helper, generic
  load/store emission, and real provider already support selected numeric vectors. The scalar-only
  resource-element classifier is the principal whole-vector transport boundary.
- Generic LLVM emission implements `swizzledStore` as one scalar store per source lane. Direct
  libNVVM can preserve those semantics with its existing byte-offset pointer, vector-extract, and
  scalar-store primitives; a dedicated callback would duplicate composition already available in
  the generic builder interface.
- The fake provider represents resource views through scalar-kind combinations. That test-only
  representation must become element-type-based before vector resource tests can remain generic.
- After vector resource storage and swizzled-store admission, the swizzle shader reached the
  distinct canonical `rwstructuredBufferLoad` opcode for each post-store reload. The read-only
  load path could be shared exactly, but RW loads must omit the invariant flag.
- Optimized PTX preserves the intended physical split: Int4 read-only traffic becomes
  `ld.global.nc.v4.u32`, Float4 RW reloads become `ld.global.v4.f32`, and the partial swizzle writes
  remain scalar `st.global.u32` instructions.

## Decision Log

- Decision: admit the complete existing selected 32-bit numeric vector family (lanes two through
  four) as structured-buffer elements instead of naming shader-specific `int4` and `float4`
  combinations.
  Rationale: value lowering and LLVM data layout already define this family uniformly. The resource
  classifier should reuse that source of truth rather than introduce another combination enum.
  Date/author: 2026-08-29, Codex.
- Decision: lower `swizzledStore` by composing existing generic builder operations.
  Rationale: canonical final IR provides a vector destination pointer, an equal-element source, and
  literal lane indices. Per-lane scalar stores exactly match established LLVM semantics and avoid
  coupling the builder ABI to a Slang IR opcode.
  Date/author: 2026-08-29, Codex.
- Decision: keep arbitrary vector pointers, dynamic lane indices, matrices, structs, half/double
  vectors, vector atomics, and helper/entry-point vector ABI out of this slice.
  Rationale: none is needed by the measured shaders, and each introduces an independent physical or
  ABI contract.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The observable result is complete. The selected vector resource family now crosses the existing
typed resource representation, both read-only and read-write canonical load opcodes, ordinary
whole-value stores, and constant-lane partial stores. No provider API or ABI changed.

The fake provider now uses one `ResourceView` parameter category and retains the exact view handle
in a parallel type list. Its existing numeric kind describes Int/Float vector pointer identity, so
the test model did not add resource-view enum cases for every lane/type combination. The positive
unit observes Int4/Float4 views, two 16-byte loads, byte offsets `12, 8, 4, 0`, six extracts, and
five stores. Double2 stops before builder discovery or mutation.

Validation evidence:

- the Release `slang-unit-test` and `slangc` targets build successfully;
- the positive, negative, and adjacent scalar-resource units pass 3/3;
- `build/Release/bin/slang-test.exe slang-unit-test-tool/nvvm` passes 362/362;
- the two selected shader files pass five CUDA/direct-PTX lanes, with outputs `0x40, 0x40, 0x37`
  and `4`;
- CUDA 12.9.86 `ptxas -arch=sm_70` accepts both optimized PTX modules and emits 2,920- and
  3,176-byte cubins.

Input-shape audit: `_getNVVMVectorSwizzledStore` is the only new production classifier. It consumes
the canonical `RWStructuredBufferGetElementPtr` and `IRSwizzledStore` produced by resource/l-value
lowering, requires the exact established vector type and literal mapping, and neither traverses an
operand graph nor rebuilds syntax. Removing vector resource admission restores the measured
conventional-field stop; removing RW load admission exposes `rwstructuredBufferLoad`; removing the
classifier exposes `swizzledStore`. These failures demonstrate that direct emission owns these
consumers. Generic LLVM emission already consumes the same shapes with scalar lane stores, so no
producer-side representation correction is indicated. The fake type-info helpers survive because
they replace scalar-only test identities with the exact types the production interface already
provides; they do not affect compiler IR.

## Context and Current Pipeline

For `StructuredBuffer<int4> input`, final linked IR stores the conventional global field as an
`HLSLStructuredBufferType<vector<int,4>>`. `getNVVMSupportedRawBufferType` currently rejects that
field through `_isNVVMSupportedResourceElementType`, even though `NVVMTypeLoweringContext` can lower
the vector and raw-buffer storage, and `kIROp_StructuredBufferLoad` can already emit an aligned
generic load.

For `RWStructuredBuffer<float4> buffer; buffer[0].wzyx = value`, final linked IR creates an
`RWStructuredBufferGetElementPtr` to `Ptr<vector<float,4>>`, then `IRSwizzledStore` with four literal
destination indices. The existing shape pass has no `swizzledStore` case, and the SSA pointer
validator currently restricts resource element pointers to whole-value loads and stores.

The principled producer/consumer boundary is final linked IR: buffer lowering owns the canonical
resource view and typed element pointer; `IRSwizzledStore` owns the exact constant destination-lane
mapping. Direct NVVM emission should consume those shapes directly. It must not reconstruct source
syntax, trace arbitrary operand graphs, or create a second type relation.

## Scope and Non-Goals

In scope:

- selected signed/unsigned integer and float32 vectors with two, three, or four lanes as exact
  structured-buffer elements;
- read-only whole-vector loads and read-write whole-vector pointer load/store transport;
- canonical one-to-four-lane `swizzledStore` with exact numeric element types and literal,
  in-range, non-repeated destination indices;
- generic fake-provider type identity for resource views;
- direct runtime, PTX, and `ptxas` lanes for the two existing shaders.

Out of scope:

- vector entry-point or helper parameters/results;
- dynamic or malformed swizzle indices, repeated lvalue lanes, or source/destination element
  conversions;
- arbitrary device vector pointers, vector atomics, shared vector storage, matrices, structs,
  half/double vectors, textures, and vector resource bounds policy.

## Architecture and Invariants

- `asNVVMSupported32BitNumericVectorType` remains the one classifier for admitted vector value and
  resource element shapes.
- `getNVVMSupportedRawBufferType`, data-pointer recognition, RW element-pointer recognition, and
  conventional-field recognition derive from the same resource-element classifier.
- A structured-buffer view lowers to `{ element addrspace(1)*, i64 count }`; its physical element
  type is exactly the canonical structured element type.
- Whole-vector memory operations use `getNVVMNumericValueAlignment`, including 8-byte two-lane and
  16-byte three/four-lane alignment.
- `swizzledStore` validation proves exact destination/source element type, literal unique indices,
  and availability before emission. Emission creates scalar pointers at `lane * 4` byte offsets and
  performs 4-byte aligned scalar stores in source-lane order.
- Unsupported shapes continue to fail preflight before provider module creation.

## Interfaces and Dependencies

No public API or `slang-llvm-nvvm` builder callback changes are planned. Production changes are
limited to the direct emitter/type-lowering implementation and existing internal classifiers. The
fake provider may replace its scalar-kind resource-view identity with generic element-type identity;
that is unit-test infrastructure, not a compatibility surface.

Validation uses the existing standalone provider under
`build/nvvm-builder-deps/slang-llvm-nvvm-build/Release`, the configured Release build, CUDA 12.9
runtime/compiler components, and `ptxas -arch=sm_70`.

## Milestones

1. Extend the resource element source of truth and focused type-lowering tests for selected vectors.
2. Add exact `IRSwizzledStore` classification, shape/SSA validation, and generic composed emission.
3. Generalize fake resource-view identity enough to trace vector views and assert per-lane stores.
4. Add direct runtime/PTX lanes to the two existing shader tests and preserve their established
   reference results.
5. Format, build, validate, update durable backend status, perform the input-shape self-review, and
   commit.

## Validation and Acceptance

Acceptance requires:

- the focused NVVM unit-test filter covering vector resource type identity, whole-vector transport,
  swizzled-store trace order, and unsupported malformed shapes;
- the complete `slang-unit-test-tool/nvvm` prefix;
- direct runtime and PTX lanes for both existing shader files;
- CUDA-scoped regression covering the changed resource family;
- `ptxas -arch=sm_70` acceptance for both emitted PTX modules;
- no builder ABI changes and no regression in existing scalar structured/byte-address resources.

All CMake builds and tests run outside the sandbox as required by repository instructions.

Executed commands (with `SLANG_NVVM_BUILDER_PATH` set to the standalone Release provider):

    cmake --build build --config Release --target slang-unit-test slangc
    build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm
    build\Release\bin\slang-test.exe -api cuda tests/compute/structured-buffer-load tests/compute/structured-buffer-swizzle-store
    build\Release\bin\slangc.exe tests\compute\structured-buffer-load.slang -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o build\slice89\structured-buffer-load.ptx
    build\Release\bin\slangc.exe tests\compute\structured-buffer-swizzle-store.slang -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o build\slice89\structured-buffer-swizzle-store.ptx
    ptxas.exe -arch=sm_70 build\slice89\structured-buffer-load.ptx -o build\slice89\structured-buffer-load.cubin
    ptxas.exe -arch=sm_70 build\slice89\structured-buffer-swizzle-store.ptx -o build\slice89\structured-buffer-swizzle-store.cubin

## Failure and Recovery

Changes are additive to the selected resource classifier and emitter switch. A failed run can be
repeated safely after rebuilding the Debug compiler and standalone provider as needed. If LLVM
rejects vector resource pointers, retain the measured IR and provider diagnostic, revert only this
slice's additive cases, and leave scalar resource support intact. Never reset unrelated work or
stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep final command counts, runtime results, representative PTX instructions, assembler results,
and the input-shape audit in this plan. Update the durable NVVM design/capability documents if the
repository already tracks these contracts. Per the user's work-loop instruction, commit this plan
only after it is complete, together with the implementation, using first commit line `slice 89`.

# Add conventional scalar uniforms and flat parameter blocks

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM route accepts a conventional CUDA global-parameter block that
combines selected scalar uniforms, a flat selected-scalar `ParameterBlock<T>`, and established
read-write structured buffers. It composes the current generic provider operations rather than
adding callbacks named for uniforms or parameter blocks.

The existing `tests/cuda/param-block-alignment.slang` gains a direct lane. CUDA/NVRTC and direct
libNVVM both produce `0, 8, 16, 8, 0, 0, 0, 0`, while direct PTX exposes the established 32-byte
host parameter block.

## Progress

- [x] (2026-08-28) Probed the existing shader and captured its final linked IR and CUDA/NVRTC PTX.
- [x] (2026-08-28) Identified the exact `{ uint, Block*, RWStructuredBuffer<uint> }` host ABI at
  offsets `0, 8, 16` with size 32 and alignment 8.
- [x] (2026-08-28) Eliminated the exact dead synthesized read-none initializer exposed by folding,
  while retaining every call with a non-null or otherwise observable pointer argument.
- [x] (2026-08-28) Admitted selected scalar conventional fields and nonempty flat selected-scalar
  parameter-block storage as a global pointer to an unpacked element struct.
- [x] (2026-08-28) Generalized exact keyed struct-field addressing and ordinary immutable loads
  across the outer conventional block and loaded parameter block.
- [x] (2026-08-28) Added structural fake-provider coverage, an unsupported nested-block negative,
  the registered direct runtime lane, direct PTX inspection, and `ptxas` validation.
- [x] (2026-08-28) Formatted with clang-format 17.0.6, completed the full Release and isolated
  provider builds, passed the complete NVVM prefix at 339/339, updated durable documentation, and
  completed the principled-change self-review.

## Surprises and Discoveries

- Slice 76 folds all four layout queries in the shader, but the now-unused `TestGlobalParams.$init`
  call remains because generic DCE conservatively treats its pointer-typed null default as
  potentially exposing memory. The helper is an exact synthesized constructor marked read-none;
  deleting only that dead call when every non-value argument is a null pointer literal preserves
  the producer contract without introducing its local `var`/store/load implementation into the
  runtime subset.
- CUDA source represents `ParameterBlock<Block>` as `Block*`. In the collected global block the
  pointer occupies eight bytes at offset 8; the resource view follows at offset 16. The provider
  already has generic struct type, pointer type, field pointer, and load operations for this graph.
- Final IR accesses a parameter-block member as `field_addr(parameterBlockValue, key)`. The loaded
  parameter-block value is already the provider pointer to its element struct, so the same generic
  field-pointer operation applies at both the outer conventional block and inner block.
- The final direct linked IR retains ordinary loads for both immutable scalar accesses. The
  existing typed load operation therefore covers the measured shader without adding a `CUDALDG`
  policy or provider semantic callback.

## Decision Log

- Decision: remove only a dead aggregate-result call whose callee is both a synthesized constructor
  and read-none and whose otherwise non-value arguments are literal null pointers, then rerun DCE.
  Rationale: the initial inlining experiment exposed non-SSA local stores that generic cleanup must
  retain conservatively. The final condition instead uses the constructor's semantic decoration
  and proves that no pointer argument exposes observable storage; broader calls remain untouched.
  Date/author: 2026-08-28, Codex.
- Decision: model a flat selected-scalar `ParameterBlock<T>` as a global-address-space pointer to
  the generic lowered storage struct for `T`.
  Rationale: this is the existing CUDA source and host ABI. It composes structural builder types
  and does not require a parameter-block-specific provider operation.
  Date/author: 2026-08-28, Codex.
- Decision: use one exact struct-field-address resolver for the conventional block and a loaded
  parameter block.
  Rationale: both shapes already carry exact `IRStructKey` identity and static field position;
  separate positional special cases would duplicate the same mapping.
  Date/author: 2026-08-28, Codex.
- Decision: keep the parameter-block element family flat and selected-scalar in this slice.
  Rationale: it proves the ABI and addressing composition needed by the existing shader while
  retaining a deterministic boundary for nested structs, arrays, resources, and opaque fields.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The existing shader now passes its unchanged CUDA/NVRTC lane and new direct libNVVM lane 2/2 with
`0, 8, 16, 8, 0, 0, 0, 0`. Direct PTX declares `.const .align 8 .b8
SLANG_globalParams[32]`, loads the resource pointer at offset 16, scalar at offset 0, block pointer
at offset 8, and the inner scalar through `ld.global.u32`. CUDA 12.9 `ptxas -arch=sm_70` accepts
the module.

The provider interface did not change. The implementation composes generic unpacked structs,
global pointers, keyed field indices, and typed loads. Fake-provider evidence records the outer
integer/pointer/resource fields, the outer resource/scalar/block indices `2/0/1`, inner index 0,
nine load results, and six stores. The unsupported nested-element source stops at the exact
conventional field-address boundary before builder discovery.

The initial dead-constructor experiment used `inlineCall`, but that correctly exposed local
non-SSA stores that generic cleanup retained. The final approach is narrower and based on producer
semantics: remove only a dead aggregate result from an exact synthesized read-none constructor
whose non-value inputs are literal null pointers. Removing this cleanup reproduces the `var`
failure; broadening runtime aggregate emission is neither necessary nor desirable for the query.

Self-review inventory:

- `asNVVMSupportedScalarParameterBlockType` survives because it defines one canonical, nonempty,
  flat element boundary and is shared by storage, value, global-field, and field-address roles.
- `_findNVVMStructField`, `_getNVVMStructFieldAddress`, and
  `_isNVVMConventionalGlobalStorageType` survive because they use exact type/key identity and keep
  one source of truth for outer and inner field mapping; they do not reconstruct syntax or walk
  arbitrary operand graphs.
- The dead-constructor condition survives because the input is the valid producer shape from
  `TestGlobalParams gp = {}` used only by folded queries, the constructor decorations are the
  semantic source of truth, and all possibly observable pointer inputs are excluded.
- No fallback, structural equivalence relation, hard-coded source field name, byte offset,
  provider callback, or compatibility path was added.

## Context and Current Pipeline

Consider the existing test:

```slang
uniform uint frame;

struct Block { uint dummy; }
ParameterBlock<Block> block;
RWStructuredBuffer<uint> outputBuffer;

outputBuffer[4] = frame;
outputBuffer[5] = block.dummy;
```

CUDA global collection produces a synthesized struct with fields `frame`, `block`, and
`outputBuffer`. The shared CUDA ABI lays them out as i32 at offset 0, a pointer at offset 8, and a
pointer/count resource view at offset 16, for a 32-byte block aligned to 8.

The outer scalar access is `field_addr(globalParams, frame)` followed by an immutable UInt load.
The parameter-block access first loads `ParameterBlock<Block>` from
`field_addr(globalParams, block)`, then forms `field_addr(parameterBlockValue, dummy)` and loads the
UInt. Provider lowering should therefore be `{ i32, { i32 } addrspace(1)*,
{ i32 addrspace(1)*, i64 } }` in addrspace(4), using field indices 0/1/2 and then inner index 0.

## Scope and Non-Goals

In scope are selected integer and float32 scalar conventional fields, nonempty flat parameter
blocks whose fields are selected integer/float32 scalars, parameter-block pointer storage, exact
outer and inner field addressing, immutable scalar loads, safe exposure of dead aggregate
initializers after layout folding, the existing parameter-block alignment shader, and adjacent
negative coverage.

Out of scope are Boolean/half/double uniform storage, nested structs, arrays, matrices, resources
inside parameter blocks, constant-buffer values beyond the compiler-synthesized outer block,
dynamic field selection, stores through uniform/parameter-block pointers, general runtime
aggregates, and any provider ABI revision.

## Architecture and Invariants

The collector and shared CUDA layout remain the sources of truth for outer field order, size, and
alignment. The conventional recognizer accepts each field structurally; executable field access
still requires an exact key, exact pointee type, and an admitted role.

A supported parameter block has an exact `IRParameterBlockType` whose element is a nonempty
`IRStructType` containing only selected integer or float32 scalar fields. Its provider value is a
global pointer to the unpacked provider struct. That representation is legal as conventional
storage and as the value loaded from it, but the element struct is not admitted as an ordinary
runtime value.

The field resolver accepts either the exact compiler-synthesized global parameter object or a
value of a supported parameter-block type. It looks up only the exact field key in the exact
element struct and validates result pointee identity. Outer sampler placeholders remain
storage-only; unsupported parameter-block fields reject the entire conventional block before
provider discovery.

The measured immutable scalar accesses retain ordinary loads and use the same provider load
construction with natural alignment. Conventional field pointers are not writable. Existing
resource-element stores retain their established mutable path.

## Interfaces and Dependencies

Extend direct NVVM type classification/lowering and emitter preflight/emission. Reuse
`eliminateDeadCode`, `IRParameterBlockType`, `IRStructField`, `IRStructKey`, generic provider struct
and pointer types, `emitStructFieldPointer`, and `emitLoad`.

Update the fake provider only enough to represent a flat element struct, a pointer to it, a mixed
outer struct, nested field pointers, and pointer-valued loads structurally. Builder ABI revision 3,
the real LLVM provider, libNVVM API, and public Slang API remain unchanged.

## Milestones

1. After folding queries, remove the exact dead synthesized read-none constructor with only
   value/null-literal inputs and prove all broader calls remain conservative.
2. Classify/lower selected scalar outer fields and flat selected-scalar parameter blocks.
3. Replace the conventional-resource-only field-address branch with one exact outer/inner struct
   resolver and add immutable scalar load handling.
4. Generalize fake structural bookkeeping and add a mixed uniform/parameter-block/resource
   positive plus unsupported nested-block negative.
5. Add the direct lane to `param-block-alignment.slang`, inspect/assemble PTX, run both GPU routes,
   update durable records, self-review, validate, and commit.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- fake emission observes outer fields i32/parameter-block-pointer/resource, outer field indices
  0/1/2, inner field index 0, pointer alignment 8, and scalar load alignment 4;
- a nested or otherwise unsupported parameter-block element stops with E52017 before builder
  discovery;
- `param-block-alignment.slang` passes CUDA/NVRTC and direct libNVVM with
  `0, 8, 16, 8, 0, 0, 0, 0`;
- direct PTX has `SLANG_globalParams[32]`, loads the outer scalar and block pointer at the expected
  offsets, reads the inner scalar, and stores all six outputs;
- CUDA 12.9 `ptxas` accepts the direct module for `sm_70`;
- the Release host build, standalone provider build, and complete NVVM prefix pass;
- formatting and `git diff --check` pass; and
- `external/slang-binaries/` and generated build artifacts remain unstaged.

## Failure and Recovery

If dead construction remains, inspect the callee decorations and arguments; do not delete an
undecorated call or one with observable pointer inputs. If ABI offsets diverge, compare the
collected IR struct and generated CUDA source before changing direct emission. Do not hard-code
byte offsets or field names.

If nested parameter blocks require aggregate value construction or a new provider operation, keep
them rejected and complete the flat proof. All implementation changes remain isolated to direct
preparation/type lowering/emission and the fake provider; reverting them restores the Slice 76
boundary.

## Artifacts and Hand-Off

Keep dumped IR, generated CUDA, PTX, `ptxas` output, and runtime logs under ignored `build/` paths.
Distill the conventional scalar/parameter-block ABI, immutable-load policy, validation evidence,
and next corpus stop into `docs/design/nvvm-backend.md` and the capability ledger. Complete this
plan's progress, outcomes, and self-review before committing it with Slice 77.

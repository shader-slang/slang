# Admit selected vector operation families

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM accepts the selected integer and float32 vector operations present
in `tests/cuda/cuda-vector-binary-ops.slang`: integer left/right shift, division, and remainder;
integer vector comparisons producing Boolean vectors; and float32 vector arithmetic and remainder.
Selected signed/unsigned integers use their exact 8/16/32/64-bit width and two through four lanes.
Float and Boolean vectors use two through four lanes. The existing shader must pass its CUDA
comparison through direct libNVVM, expose representative integer and floating instructions in PTX,
and pass CUDA 12.9 `ptxas -arch=sm_70`.

## Progress

- [x] (2026-08-29) Completed and committed Slice 86 as `6fee23603` with 356/356 NVVM tests.
- [x] (2026-08-29) Captured the existing shader's final linked IR and its exact operation/type
  census.
- [x] (2026-08-29) Chose one typed operation-family slice because LLVM models floating remainder
  directly and no libdevice contract divides the graph.
- [x] (2026-08-29) Generalized ordinary SSA vector classification, type lowering, construction,
  extraction, and fake identity without broadening memory/resource payload policy.
- [x] (2026-08-29) Added typed shift and remainder operation codes and expanded integer
  comparison/binary and float-binary semantic families.
- [x] (2026-08-29) Emitted exact LLVM signed/unsigned shift, division, remainder, comparison, and
  floating arithmetic instructions in the real provider.
- [x] (2026-08-29) Added focused real-provider and fake-emitter coverage and registered the
  existing shader for direct runtime/PTX evidence.
- [x] (2026-08-29) Formatted, built, ran focused and complete validation, updated durable
  documents, self-reviewed, and prepared this plan with the implementation for commit.

## Surprises and Discoveries

- The final graph remains vectorized. Its operation census is four `add`, one `sub`, six `mul`, one
  `and`, one `shl`, two `shr`, one `div`, one `irem`, one `frem`, one vector `cmpEQ`, one vector
  `cmpLT`, seventeen vector constructors, and thirty-seven scalar swizzles.
- Floating `%` reaches the direct boundary as canonical `frem`, which LLVM 14 represents with
  `IRBuilder::CreateFRem`. It is not a libdevice call and therefore does not justify a separate
  compatibility slice.
- The shader deliberately uses signed `Int8x2` for right shift, division, remainder, and comparison.
  Semantic signedness must select `ashr`, `sdiv`, `srem`, and signed predicates even though LLVM
  integer vector types themselves are signless.
- The shader's operands are all constants. libNVVM accepts the complete vector graph, then folds it
  to literal global stores in final PTX. Opcode evidence therefore belongs in the unoptimized
  normal/compatible provider assembly; the file-backed runtime comparison is the lane-order and
  signed-result oracle.
- The first fake-emitter run stopped after the float-vector addition because its historical
  eight-slot float-constant fixture could not represent the ninth distinct asymmetric value.
  Expanding that generated-test storage to the same 64-slot capacity as integer constants removed
  the artificial ceiling without changing production behavior.

## Decision Log

- Decision: keep byte-address, structured-buffer, entry-parameter, and device-pointer vector roles
  on their existing exact policies while adding a broader classifier for ordinary SSA vector
  values.
  Rationale: the motivating final graph constructs, computes, compares, and extracts vectors but
  never transports the new narrow/Boolean shapes through memory or ABI boundaries. Broadening those
  independent roles would claim unmeasured layout and access contracts.
  Date/author: 2026-08-29, Codex.
- Decision: append semantic operation IDs for remainder, left shift, and right shift and route them
  through the existing descriptor-based generic operation callback.
  Rationale: division already proves that signedness belongs in the typed descriptor rather than
  in a new callback. The same callback can select all LLVM opcodes without another builder ABI
  revision or a combinatorial overload enum.
  Date/author: 2026-08-29, Codex.
- Decision: represent comparison results as the same bounded lane count in the semantic Boolean
  descriptor.
  Rationale: LLVM `icmp` naturally returns `<N x i1>` for `<N x iW>` inputs, and the final Slang IR
  consumes that exact Boolean vector through ordinary scalar extraction. Scalar comparison remains
  the lane-one member of the same family.
  Date/author: 2026-08-29, Codex.
- Decision: add one float32 binary family covering two through four lanes, including `frem`, while
  leaving the frozen scalar catalog rows intact.
  Rationale: this extends the current generic descriptor architecture forward without duplicating
  every width/lane overload. Exact scalar rows remain the established source for scalar diagnostic
  names and GenericAsm matching.
  Date/author: 2026-08-29, Codex.
- Decision: represent Boolean vector identity in the fake provider as the generic element kind plus
  lane count rather than adding `Bool2`, `Bool3`, and `Bool4` cases to its older test enum.
  Rationale: the production API is dimensioned, and the fake should not recreate the combinatorial
  type interface this backend was explicitly generalized to avoid.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The selected vector graph now reaches libNVVM through one dimensioned descriptor architecture.
The real provider's normal LLVM 14 and NVVM-compatible text contain representative signed and
unsigned shifts, signed narrow division/remainder/comparison, Boolean vector results, and float32
vector addition/remainder. The fake boundary observes exact semantic result descriptors and the
constructor/operation/extractor producer-consumer chain. All four former scalar unsupported
controls compile through the same operation family, while the remaining unsupported matrix still
stops before provider discovery.

`tests/cuda/cuda-vector-binary-ops.slang` passes CPU, CUDA/NVRTC, direct-libNVVM runtime, and direct
PTX lanes 4/4. Its established 37 results cover asymmetric lanes, signed `Int8x2`, Boolean
extraction, and floating remainder. CUDA 12.9.86 `ptxas -arch=sm_70` accepts the 2,143-byte direct
PTX and produces a 3,688-byte cubin. Focused/adjacent units pass 4/4. Pinned clang-format 17 and
`git diff --check` pass. The standalone provider and Release `slang-unit-test`, `slang-test`, and
`slangc` targets build, and the complete NVVM prefix passes 358/358.

The final self-review inventory found four intentional generalizations and no fallback. The private
ordinary-vector classifier is the single source for canonical literal lane counts and selected
element types; numeric, 32-bit byte/memory, and signed/unsigned-i32 helpers narrow it by role. The
semantic-family branches consume the exact final `IRVectorType` descriptors produced by ordinary
CUDA legalization and require identical result/operand widths and lanes, so they do not recover an
alternate representation. The provider's signed opcode choice consumes the descriptor's existing
semantic signedness rather than inspecting source names or rebuilding types. The fake-provider
changes validate descriptor-driven operands, represent Boolean vectors by generic element kind and
lane count, and raise only generated-fixture storage from eight to 64 float constants. No graph
walk, source spelling, scalarization, text rewrite, compatibility branch, or consumer-side repair
remains.

Final commands and observed evidence:

- `cmake --build build\nvvm-builder-deps\slang-llvm-nvvm-build --config Release --target slang-llvm-nvvm`
  built the standalone provider.
- `cmake --build build --config Release --target slang-unit-test slang-test slangc` built all
  selected Release host targets.
- `build\Release\bin\slang-test.exe slang-unit-test-tool/nvvmIRBuilderBuildsNumericTypeFamilies slang-unit-test-tool/nvvmSlangVectorOperationFamiliesUseTypedDescriptors slang-unit-test-tool/nvvmSlangScalarShiftDivideRemainderUseTypedOperations slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission`
  passed 4/4.
- `build\Release\bin\slang-test.exe tests/cuda/cuda-vector-binary-ops` passed 4/4.
- `build\Release\bin\slang-test.exe slang-unit-test-tool/nvvm` passed 358/358.
- CUDA 12.9.86 `ptxas.exe -arch=sm_70 build\nvvm-slice87\vector-ops.ptx` succeeded.

## Context and Current Pipeline

Consider the existing source:

```slang
int2 shl2 = int2(7, 3) << int2(2, 5);
int8_t2 c2 = int8_t2(-6, 7);
int8_t2 cshr2 = c2 >> int8_t2(1, 1);
int8_t2 cdiv2 = c2 / int8_t2(2, 2);
int8_t2 cmod2 = c2 % int8_t2(4, 4);
bool2 cneg2 = c2 < int8_t2(0, 0);
float3 fmod3 = float3(7.5, -7.5, 8.5) % float3(2.0, 2.0, 3.0);
```

CUDA legalization leaves those operations as exact vector `shl`, `shr`, `div`, `irem`, `cmpLT`,
and `frem` instructions. `_preflightNVVMIR` currently stops at the first `shl` because
`_getNVVMValueOperation` has no shift/remainder mapping, `_getNVVMSemanticType` names only 32-bit
integer vectors, and the semantic catalog admits neither these binary operations nor vector result
comparisons. Vector construction/extraction is separately limited to 32-bit numeric vectors.

After preflight, `_resolveNVVMValueOperation` carries exact result and operand descriptors through
the generic `emitValueOperation` callback. The LLVM provider resolves the same catalog/family and
creates one typed instruction. Vector construction/extraction already use generic provider
operations, so the producer-side fix is to broaden their ordinary-SSA type classifier rather than
scalarize the graph in the emitter.

## Scope and Non-Goals

In scope are exact two- through four-lane selected integer, float32, and Boolean SSA vectors;
numeric vector construction; selected vector extraction; integer shift/divide/remainder;
integer vector comparison; float32 vector add/subtract/multiply/divide/remainder; typed capability
preflight; real/fake provider evidence; and the named existing shader's direct runtime/PTX lanes.

Out of scope are half/double; vectors wider than four lanes; Boolean construction or arithmetic;
vector phis, helper/entry ABI, local/global/shared memory, device pointers, structured-buffer or
byte-address payload expansion; dynamic vector indexing; matrix operations; floating comparison;
fast-math flags; division-by-zero or overshift semantic repair; and new source legalization.

## Architecture and Invariants

One ordinary SSA-vector classifier accepts a canonical `IRVectorType`, literal lane count two
through four, and an exact selected scalar element. A numeric narrowing excludes Boolean for
constructors and arithmetic. Existing 32-bit numeric and signed/unsigned i32 classifiers narrow
the broader family for memory and pointer roles, so those contracts do not change accidentally.

The semantic descriptor preserves kind, bit width, and lane count. Binary numeric operations
require result and both operands to match exactly. Integer comparison requires matching selected
integer operands and a Boolean result with the same lane count. Signedness is semantic metadata:
the provider maps it to `ashr`/`lshr`, `sdiv`/`udiv`, `srem`/`urem`, and signed/unsigned predicates.
Float32 operations map directly to LLVM `fadd`, `fsub`, `fmul`, `fdiv`, and `frem` with no inferred
flags.

Vector constructors receive exact scalar elements; extraction receives a constant in-range lane
and returns the exact element type. Boolean vectors are operation results and extraction bases, not
new source constructors. Every value must still be usable at the insertion point and belong to the
same provider module.

## Interfaces and Dependencies

Append remainder/left-shift/right-shift IDs to
`source/compiler-core/slang-nvvm-ir-builder-api.h` and update the operation count. Expand only the
generic semantic resolver in `slang-nvvm-semantic-catalog.h`; no callback table member, feature
constant, compatibility shim, structure-size field, or builder ABI revision is added.

Update ordinary vector classifiers and lowering under `source/slang/`, the LLVM operation/type
selection in `source/slang-llvm-nvvm/`, and the fake provider plus focused tests under
`tools/slang-unit-test/`. Register direct lanes in the existing shader. libNVVM and CUDA 12.9
`ptxas` remain the external acceptance boundary.

## Milestones

1. Add the canonical selected ordinary-vector classifier and preserve narrower memory-role helpers.
2. Extend semantic operation mapping/families and provider LLVM instruction selection.
3. Generalize vector construction/extraction and fake value identity for the admitted SSA shapes.
4. Add focused provider/fake tests and remove only the four newly supported scalar operations from
   the unsupported matrix.
5. Register the existing shader's direct CUDA comparison and PTX lane; compile, inspect, execute,
   and run `ptxas`.
6. Run focused regressions and the complete NVVM prefix, update durable design/ledger records,
   self-review, and commit this plan with the implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires:

- semantic-family tests accept exact selected scalar/vector descriptors and reject mismatched
  signedness, width, lane count, Boolean arithmetic, and unsupported float widths;
- real-provider assembly contains representative `<2 x i32>` shifts, `<2 x i8>` signed
  shift/divide/remainder/comparison, `<3 x i1>` comparison, and `<3 x float>` add/remainder;
- fake emission records exact descriptors, constructors, operation-result identities, and scalar
  extracts before serialization/libNVVM handoff;
- the former scalar shift/divide/remainder unsupported controls now compile through the generic
  family, while adjacent logical-not/libdevice/pointer/atomic controls remain pre-provider E52017;
- `tests/cuda/cuda-vector-binary-ops.slang` passes CUDA/NVRTC and direct-libNVVM runtime comparison
  and direct PTX checking;
- CUDA 12.9 `ptxas -arch=sm_70` accepts direct PTX;
- standalone provider and Release `slang-unit-test`, `slang-test`, and `slangc` targets build;
- focused unit/file tests, adjacent unsupported tests, and the complete NVVM prefix pass; and
- pinned clang-format 17 and `git diff --check` pass.

Record exact commands and observed counts in this plan before commit.

## Failure and Recovery

All source edits are additive/generalizing and rerunnable. If LLVM verification or libNVVM rejects
one opcode, retain the type/semantic evidence for the accepted families and split only at the
external contract that failed; do not replace a rejected vector instruction with text surgery or
emitter-side scalarization without a new measured plan. If runtime differs, compare final linked
IR, normal LLVM text, NVVM-2.0 text, PTX, and the asymmetric expected lanes to locate signedness or
lane-order loss. Reverting the slice restores deterministic E52017 at the first shift without
affecting the established backend.

## Artifacts and Hand-Off

Keep final linked-IR census, LLVM/NVVM assembly, PTX, and `ptxas` evidence under ignored
`build/nvvm-slice87/`. Distill stable ordinary-vector and operation-family contracts into
`docs/design/nvvm-backend.md`, add suite coverage to the capability ledger, and include this
completed plan in the slice commit as explicitly requested.

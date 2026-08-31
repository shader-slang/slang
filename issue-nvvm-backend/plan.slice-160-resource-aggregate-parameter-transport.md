# Transport canonical resource aggregates across compute parameter boundaries

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, a finite resource-bearing struct uses one established provider aggregate
representation across raw CUDA launch parameters and collected conventional CUDA global
parameters. The bounded target set is the four frozen-v1 and two discovery workloads whose first
blocker is an entry parameter or collected `global_param` carrying specialized interface/resource
data. Correct targets gain permanent direct O0/O3 lanes only after differential execution.

## Progress

- [x] (2026-08-31) Re-ranked the Slice 159 healthy failures across both corpora and selected the
  cross-corpus aggregate parameter-boundary cluster.
- [x] (2026-08-31) Audited the established scalar `byval` launch ABI, conventional constant-memory
  block, recursive resource-struct classifier, and generic provider operations.
- [x] (2026-08-31) Captured each target's exact final type and distinguished raw launch parameters from collected
  conventional storage without merging their physical roles.
- [x] (2026-08-31) Generalized the shared finite-resource representation through both parameter boundaries and
  exact keyed field access, without changing provider ABI revision 30.
- [x] (2026-08-31) Built and probed all six targets at O0/O3, promoted five correct rows, and
  recorded the independent parameter-group entry ABI cascade on the sixth.
- [x] (2026-08-31) Ran the selected prefix, frozen/discovery corpora, representative measurements, formatting,
  integrity checks, and self-review.
- [x] (2026-08-31) Completed durable documentation, this plan, and the five-part report for the
  Slice 160 commit.

## Surprises and Discoveries

- Slice 80 already established that an aggregate raw CUDA launch parameter must be represented as
  a generic LLVM pointer carrying exact `byval` pointee and CUDA alignment attributes. The same
  Slang struct remains first-class in ordinary value roles.
- Slice 127 established `asNVVMSupportedResourceStructType` as the recursive, cycle-safe source of
  truth for finite structs containing selected numeric and resource leaves. Later slices extended
  that same provider struct through helpers, results, locals, structured-buffer storage, and
  explicit construction; only parameter boundaries remain artificially numeric-only.
- Collected conventional globals are physically different from launch parameters: the synthesized
  outer struct is declared in LLVM constant address space and field loads use keyed GEP plus
  invariant load. Nested resource aggregates should stay stored by value there rather than acquire
  the raw launch parameter's `byval` pointer wrapper.
- The first raw-entry probe reached provider `generic value call`: mapping the physical `byval`
  pointer directly to the semantic Slang `IRParam` works for field GEPs but is not a valid argument
  for a helper expecting the aggregate value. Retaining the physical pointer separately and
  materializing one invariant semantic aggregate value repairs the representation boundary.
- Discovery `generic-shader-object-cbuffer2` remains at entry-parameter preflight because its
  parameter is the `ParameterBlock<Impl<...>>` wrapper itself, not the selected finite resource
  struct. That producer requires a separate parameter-group launch ABI and remains out of scope.
- The old adjacent-shape negative contained `Outer { Inner { uint value; }; }`. The recursive
  resource-struct classifier intentionally admits that finite canonical aggregate, so expecting
  preflight rejection became stale. The fake provider cannot distinguish all nested generic struct
  handles and therefore cannot provide useful positive evidence for this shape. A real LLVM 14
  provider probe passes at O0 and O3, including whole-value transport to a helper, and both PTX
  outputs assemble for SM70. The obsolete fake negative was removed; the incompatible-layout and
  unsupported matrix-operation negatives remain.
- An initial full-prefix run appeared to fail broadly because the provider path was supplied as an
  unsupported tool-environment property rather than in the PowerShell process. With
  `SLANG_NVVM_BUILDER_PATH` set in-process, the ABI-30 provider passes the complete prefix. This was
  validation setup, not a compiler or provider defect.

## Decision Log

- Decision: reuse `asNVVMSupportedResourceStructType` at both parameter boundaries instead of
  adding interface-, fixture-, or resource-kind-specific classifiers.
  Rationale: specialization has already removed interface semantics; final IR carries one finite
  canonical struct recursively composed of already selected provider values.
  Date/author: 2026-08-31, Codex.
- Decision: preserve distinct physical roles for raw launch and conventional collected storage.
  Rationale: raw aggregate launch parameters follow NVPTX `byval`, while the synthesized global
  block is ordinary constant-address-space storage. Both contain the same exact value struct but
  cannot share the pointer wrapper.
  Date/author: 2026-08-31, Codex.
- Decision: require complete CUDA/LLVM field-offset and size agreement before admitting either
  resource aggregate boundary.
  Rationale: generic LLVM structs are correct only when the producer's CUDA layout matches their
  unpadded physical field layout; otherwise legalization must establish a different canonical
  representation upstream.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Five of the six selected workloads are correct at direct O0 and O3. The four frozen rows gain eight
permanent lanes, and discovery `interface-shader-param` gains two. Frozen corpus v1 remains exactly
452 workloads/427 healthy references and improves from 386/390/386 to 390/394/390 O0/O3/both-mode
correctness, with zero old-correct loss. Across all frozen rows, native CUDA is 449 correct/three
infrastructure; direct O0 is 403 correct, 36 preflight, eight runtime mismatch, and five provider;
direct O3 is 408 correct, 36 preflight, and eight runtime mismatch.

Discovery remains exactly 82 workloads/72 healthy references and improves from 60/60/60 to
61/61/61, with zero old-correct loss. Each direct mode has 61 correct, 11 preflight, two provider,
seven infrastructure, and one runtime mismatch. The remaining `generic-shader-object-cbuffer2`
row proves a separate parameter-group launch ABI and is not counted as unlocked.

The selected prefix passes 427/427. All seventeen representative direct-O3 gates assemble with
CUDA 12.9 for SM70, SM80, and SM90. The new conventional aggregate/helper gate measures 258.1 ms
and 797-byte PTX at direct O3 SM70 versus 370.3 ms and 8770 bytes through NVRTC O3; direct O0
measures 255.1 ms and emits 22133-byte PTX. These measurements remain exploratory.

The repository formatter was attempted with `--modified`, but this machine does not provide
gersemi, clang-format, prettier, or shfmt. Manual review, `git diff --check`, JSON parsing, exact
TSV identity/count checks, and measurement completeness checks pass.

The implementation needed no provider callback or ABI revision. The decisive representation rule
is to keep a raw aggregate launch parameter's physical `byval` pointer separate from its semantic
first-class value. One invariant load supplies helper calls, while direct entry-field extraction
continues to use the physical pointer. Conventional collected storage remains an immutable
constant-address-space aggregate and uses its existing keyed field path.

## Context and Current Pipeline

The current entry-parameter classifier admits selected scalars, numeric pointers/arrays, raw
buffers, and flat scalar structs. Scalar structs lower to a role-specific generic pointer; the
function parameter receives `BY_VALUE`, exact aggregate pointee, and CUDA alignment attributes.
Entry-block field extraction recognizes only that scalar classifier and performs typed GEP plus
invariant load.

The conventional parameter collector produces a synthesized `ConstantBuffer<GlobalParams>`.
`_getNVVMConventionalGlobalParams` verifies every outer field, declares one constant-memory LLVM
struct, and `_getNVVMStructFieldAddress` resolves fields by semantic key. Its accepted field list
contains individual resources and parameter groups but still excludes an ordinary nested
resource-bearing value struct.

The selected workloads are:

- frozen `compute/interface-assoc-type-param.slang` and
  `compute/interface-func-param-in-struct.slang`, currently rejected at an entry parameter;
- discovery `compute/interface-shader-param.slang` and
  `language-feature/generics/generic-shader-object-cbuffer2.slang`, also rejected at an entry
  parameter; and
- frozen `compute/simple-interface-parameter.slang` and `cuda/copy-elision-this-2.slang`, whose
  collected conventional `global_param` is rejected before field use.

## Scope and Non-Goals

In scope are exact finite resource structs at raw entry parameters; `byval` declaration,
attributes, and entry-block field loads; exact finite resource structs as fields of the one
canonical synthesized global parameter block; recursive compatible-layout validation; keyed
field addresses and invariant loads; focused fake/real-provider evidence where useful; the six
selected corpus rows; complete cross-corpus validation; and representative measurements.

Out of scope are arbitrary unspecialized interfaces, witness values, malformed upstream IR,
source reconstruction, aggregate flattening, new provider callbacks, provider ABI changes,
helper aggregate parameters beyond the established contract, arbitrary entry arrays, resource
arrays that lack an established value representation, pointer address-space casts, entry-point
parameter blocks with a different producer, fixture-name checks, fallbacks, and corpus-v2
activation.

## Architecture and Invariants

- The recursive resource-struct classifier is the only admitted value algebra. It must reject
  cycles and unsupported leaves before provider mutation.
- A raw resource-aggregate launch parameter has a role-specific generic pointer representation and
  exact `byval` pointee/alignment attributes; the canonical ordinary value remains an LLVM struct.
- A collected resource aggregate remains a field in the synthesized constant-memory struct and is
  loaded by value. It never receives a launch `byval` wrapper.
- Field identity comes from the canonical struct key and declared position. Layout validation
  compares CUDA and LLVM size and every field offset recursively.
- All construction uses existing generic struct, pointer, function-attribute, GEP, and load
  operations. Provider ABI revision 30 remains unchanged.
- Frozen corpus v1 remains exactly 452/427 and discovery exactly 82/72, reported separately with
  zero old-correct regression required.

## Interfaces and Dependencies

Production work is expected in `source/slang/slang-emit-nvvm-type-lowering.cpp` and
`source/slang/slang-emit-nvvm.cpp`. Focused fake evidence may touch the existing NVVM unit-test
support/emitter files. Correct existing workloads may gain direct O0/O3 directives after
differential validation. Census, Pareto, measurement, design, ledger, plan, and report artifacts
are retained under their established locations.

## Milestones

1. Replace the scalar-only launch-aggregate role checks with the existing exact finite resource
   struct classifier, preserving the entry-parameter representation cache and generic `byval`
   contract. Generalize entry-block field loads to exact resource field alignment.
2. Admit a finite resource struct as one field of the synthesized conventional block, validate its
   recursive CUDA/LLVM layout, lower it in storage role, and retain keyed immutable field access.
3. Build and run all six targets through healthy NVRTC and direct NVVM O0/O3. Promote only rows
   that are fully correct; record any independent next blocker without speculative widening.
4. Run promoted files, the selected 427-test prefix, complete frozen/discovery corpora, and the
   representative SM70/80/90 assembly matrix. Update durable documentation and complete the
   input-shape/self-review audit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools. Acceptance requires
exact fake or real-provider evidence for the raw `byval` resource aggregate and the stored
conventional resource aggregate; differential O0/O3 evidence for every promoted target; zero
old-correct regression; the 427-test prefix; unchanged exact corpus identities; PTX assembly for
SM70, SM80, and SM90 where practical; provider ABI revision 30; formatting attempt;
`git diff --check`; JSON/TSV integrity; and an exact staged-file audit excluding
`external/slang-binaries/`.

## Failure and Recovery

If a target reaches a separate operation, retain the general parameter representation and record
the exact new producer/type/diagnostic without widening this slice. If CUDA and LLVM layouts
disagree, stop at a typed layout diagnostic and plan producer-side legalization; do not pack LLVM
types manually or patch emitted text. Generated IR, logs, PTX, and cubins remain reproducible
under ignored `build/` paths.

## Artifacts and Hand-Off

Commit this completed plan with the implementation because the user explicitly requires it.
Retain Slice 160 frozen/discovery TSV and Pareto JSON, any refreshed measurement manifest, the
five-part report, promoted lanes, design/ledger updates, and focused tests. Raw diagnostic dumps
remain under `build/`.

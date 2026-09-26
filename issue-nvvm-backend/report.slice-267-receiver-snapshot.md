# Narrow NVVM helper parameters without losing value snapshots

## Motivation

Research266 preserves a material-derived case where NVRTC removes constant absorption exponentials
but NVVM retains them. A nonmutating normal helper receives a complete Graph snapshot although it
reads only two fields. Consider its body and caller:

```slang
float3 adjust(float3 normal)
{
    if (hints == 0)
        return normal;
    return normal + direction;
}

[mutating]
void prepare(uint index, float3 normal)
{
    stack.normals[index] = adjust(normal);
}
```

`prepare` snapshots Graph before calling `adjust`. Other fields contain initialized absorption,
counters and arrays; the helper does not read them. The [complete reproducer](experiments/material-reproducer/graph.slangh)
checks both absorption and array outputs. This experiment tests whether exposing consumed fields
at the call boundary improves the unchanged reproducer and original material.

## Proposed solution

Replace eligible internal struct value parameters with the fields their callees extract. Extract
arguments from the original SSA snapshot, preserving value semantics across later mutation. Reuse
existing callee-first traversal, canonical field keys, builders and function-type repair, then let
`deferBufferLoad` enforce its existing memory-stability proof. Schedule the transformation only for
direct NVVM, with NVVM's existing helper-value type policy determining which fields can become
standalone parameters. [Validation267](runtime-validation.slice-267.json) accepts the final compiler
after full correctness preservation and the declared resource/timing gates.

## Change summary

`slang-ir-transform-params-to-constref.{cpp,h}` adds the field transformation and shares existing
function traversal/direct-call eligibility. `slang-emit.cpp` supplies `isNVVMSupportedHelperValueType`
and schedules it before load narrowing. Two CUDA fixtures cover ordinary snapshots and resource
aggregates; [source counterfactuals](experiments/receiver-snapshot/README.md) retain the original
branch while narrowing its inputs. The plan, structured evidence, design note and navigation record
this bounded slice. Raw attempts, patches, binaries, dumps and samples stay under
`build/nvvm-receiver-snapshot267`.

## Concepts and vocabulary

A receiver snapshot is a value independent of later writes to its source object. A canonical field
key identifies a struct field throughout Slang IR. Value-parameter decomposition passes selected
field values without creating references to their storage. NVVM's helper-value domain is its existing
recursive classification of values supported as ordinary internal function parameters; a resource
may be supported inside an aggregate without being supported as a standalone parameter.

## Process report

Explicit hints/direction inputs retain the helper branch and change constant NVRTC/NVVM O3
exponential counts from 0/3 to 0/0, and runtime absorption counts from 2/6 to 2/2. Twelve original/narrow
GPU cells pass. Six more cells execute both hint branches, with independently expected outputs
`[12, 1, 1, 1, 6, 4, 4, 1]`. Passing SurfaceInteraction/hints explicitly to the material helper removes
its six NVVM exponentials in both entries. These comparisons establish the hypothesis, not a compiler
optimization or GPU-speed result.

The original input is canonical: implicit copyable receivers use input value passing;
`addCallArgsForParam` obtains an rvalue and `getSimpleVal` loads the aggregate. Existing immutable-buffer
argument specialization deliberately excludes local/inout roots. Replacing the snapshot with its
source address or delaying reads across mutation would violate that contract. This transformation
instead changes the internal value interface. It narrows the material's policy query first, making
the caller's field uses visible, then narrows the normal helper.

The helper inventory comprises a shared direct-call predicate, conservative function eligibility,
`collectFields`, `processFunc`, and the scheduling entry point. `collectFields` accepts only ordinary,
undecorated struct parameters used through direct field extracts selecting a strict subset of fields.
Every selected field must satisfy the target's existing type policy. Whole-value uses, external
signatures, unknown uses and signature-bearing metadata remain intact. Semantic lookup uses field
keys; declaration order only stabilizes the new signature. `processFunc` uses `createParam`, explicit
insertion, canonical extraction and `fixUpFuncType`, preserving call decorations and source locations.
There is no custom equivalence, new alias claim, syntax reconstruction or provider change.

An inactive branch's extracted value remains harmless: ordinary SSA aggregates use LLVM
`extractvalue`; pointer-backed entry aggregates use their valid by-value storage. Extraction does not
dereference an extracted pointer, and helper parameters/calls add no `noundef` contract. Independent
review checked `_emitAggregateElementExtract`, `_declareFunction` and `_emitCall`. Existing load
narrowing remains responsible for memory-motion proofs.

The first full checkpoint exposed the necessary type-policy boundary. It preserved all 1356 frozen
outcomes but regressed `bindings/nested-parameter-block-3.slang` in NVVM O0/O3: narrowing introduced a
bare `ParameterBlock<MaterialSystem>` helper parameter after resource specialization. Moving the
pass earlier fixed that global-root case, but a [runtime-selected local resource snapshot](../tests/cuda/nvvm-aggregate-param-resource-snapshot.slang)
still failed. Its baseline passes all three GPU modes with outputs 7/11. Existing specialization
intentionally excludes its local/phi root; reordering alone therefore cannot make the new interface
legal. The final policy reuses `isNVVMSupportedHelperValueType` through a required callback, keeping
the original aggregate when any selected field is outside that domain. `isSimpleDataType` was
rejected because it admits arbitrary pointers; no second recursive classifier or provenance walk
was added. The earlier scheduling change became unnecessary and was removed. The valid aggregate
producer needs no repair: this optimization owns the legality of its new interface.

The corrected candidate passes 18 standalone compile/assembly cells and 31 focused runtime/IR checks.
Unchanged masked/unmasked constants lose all three NVVM exponentials; controls retain two. Both
unchanged material entries lose all six exponentials, with these NVVM O3 entry resources:

| Entry           | Registers before / after | Stack bytes before / after | Spill bytes before / after |
| --------------- | ------------------------ | -------------------------- | -------------------------- |
| `eval_buffer`   | 67 / 52                  | 784 / 0                    | 0 / 0                      |
| `sample_buffer` | 86 / 62                  | 784 / 0                    | 0 / 0                      |

The common-`deferBufferLoad` regression check
fails the baseline specifically for its original whole-struct signature while its three GPU modes
pass; the candidate passes all four checks. IR review confirms matching parameter/call order,
snapshots surviving writes 99/88, first/last field mapping, unchanged whole-value return, and the
selected resource aggregate remaining intact. The branch-selected ordinary test proves selection 20/40;
its unobserved later writes can be eliminated and are not claimed as separate mutation coverage.

All failed attempts remain recorded. Initial `emitParam` calls appended parameters despite the
insertion point, causing E52017 argument-order rejection; `createParam` plus explicit insertion fixes
that producer mistake. A same-object value/inout test violated Slang's nonalias contract and produced
E30051; both it and its warning-suppressed diagnostic experiment are excluded. A missing-output IR-test
command was corrected before qualification. Two overlapping incremental builds were excluded and
followed by a forced serialized rebuild. The rejected scheduling-only build retains its exact patch,
build and shader logs, but its transient binary hashes were not captured; final candidate identities
are complete. The first rejected full checkpoint is preserved rather than becoming a baseline.

A separate source experiment with an explicit copied constref snapshot fails accepted NVVM with
E52018, compact parameter-group vector extraction. A 16-line float3-containing struct reproduces it;
float4 succeeds. `_getNVVMStructFieldAddress` recognizes a borrowed helper field, but
`_getNVVMCompactParameterGroupVectorPointer` mistakes that read-only field for compact storage even
though its helper layout uses a native vector. This separate correctness issue remains queued.

Final correctness preserves all 1356 frozen and 357 discovery outcomes, 567 input hashes, 39 known
gaps and 18 resolved histories. Units retain 1086 pass/13 skip and semantics 1170 pass/78 skip with
exact test identities. Runtime4, material6, toolkit18 and harness contracts all pass. Lead and
independent review accept these results for quality measurement.

The fixed 36-cell quality subset preserves all 64 named-function resource records exactly.
Two isolated material timing rounds reverse both case and compiler order, with two warmups and
nine measured samples per case/compiler/round. All 264 compile and 132 assembly attempts succeed;
output hashes are stable. NVVM O3 fresh-process medians are:

| Entry           | Round 1 baseline / candidate | Round 2 baseline / candidate |
| --------------- | ---------------------------- | ---------------------------- |
| `eval_buffer`   | 1375.08 / 1348.69 ms         | 1368.12 / 1348.22 ms         |
| `sample_buffer` | 1473.58 / 1441.15 ms         | 1471.69 / 1437.11 ms         |

All four comparisons meet the declared limit of no more than 5% slowdown. The observed reductions
are 1.45–2.35%; this is a bounded compile-time result, not a general compiler or GPU-speed claim.
Fresh processes share warmed filesystem/toolkit/PCH caches. Standard nested Slang timers are retained;
this run does not separately attribute vendor phases.

At O0, evaluation stack grows from 2960 to 3280 bytes and sampling from 3200 to 3520 bytes, with
unchanged 128/168 registers. Four additional helper blocks remain in each entry's module, all with
zero stack and spills; helper register counts are unavailable. The transformation creates parameters
and calls, not functions; both baseline and candidate LLVM contain exactly one internal definition
of each flagged helper. PTX therefore shows changed retention/inlining of existing helpers,
without identifying a particular downstream pass. O3 removes these helpers. NVRTC resources
remain unchanged, and no mode adds spills. The fail-closed resource checker flags the changed O0
symbol sets and exits nonzero; that raw result remains intact for explicit acceptance review.

Lead and independent review accept this O0 tradeoff: the declared six-exponential/784-byte stack
benefit concerns O3, while the all-mode no-new-spill and per-round timing gates pass. The raw nonzero
resource-review result is preserved alongside the separate reviewed acceptance, with no rerun or
sample removal. O0 modules grow and O3 modules shrink; exact PTX/cubin sizes are in the ledger.
The compiler change is accepted and this bounded experiment is complete.
Material runtime remains unassessed because binding,
texture/LUT, input and expected-output contracts are unavailable. The general loop remains stopped.

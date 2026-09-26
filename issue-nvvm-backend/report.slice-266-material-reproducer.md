# Reproduce the material's lost aggregate constants

## Motivation

The unchanged tiled-brass material has six exponential instructions in each NVVM O3 entry and none
in NVRTC O3. Research264 linked them to absorption fields initialized to zero, but its general probe
lost constants in both backends. We needed a runnable example that preserves the difference before
choosing a compiler transformation.

Consider the complete research program in [graph.slangh](experiments/material-reproducer/graph.slangh).
Its entry observes both the absorption calculation and the constructed arrays:

```slang
[numthreads(1, 1, 1)]
void computeMain()
{
    Graph graph = makeGraph(float3(inputs[0], inputs[1], inputs[2]), 0);
    outputs[0] = uint(dot(exp2(-graph.stack.payload.absorption), float3(8)));
    outputs[1] = uint(graph.stack.normals[0].z);
    outputs[2] = uint(graph.stack.normals[1].z);
    outputs[3] = uint(graph.stack.layers[0].x);
}
```

`makeGraph` initializes every field, then `populate` writes zero absorption, prepares normals at
counter-derived indices zero and one, and writes layer zero. The expected result is therefore
24, 1, 1, 1. A second entry uses runtime absorption values one and two, producing 12 and 6 while
preserving the array checks. Those arithmetic expectations do not depend on a reference compiler.

## Proposed solution

Keep a small explanatory reproducer and two controlled source variants, with exact-entry runtime
tests and compiler dumps. Change no compiler, provider, original material, or accepted inventory.
The result establishes an optimization boundary; it does not select a production optimization.

## Change summary

[The fixture README](experiments/material-reproducer/README.md) documents commands, outputs,
controls and source-to-generated-code evidence. The `masked`, `unmasked` and `branchless` constant
and control wrappers share one graph definition and test their `computeMain` entry in three modes.
The completed plan and structured record retain provenance and outcomes; STATUS/HANDOFF/HISTORY and the
material design note carry the findings forward. Raw reductions and full dumps stay under
`build/nvvm-material-reproducer266`.

## Concepts and vocabulary

A nonmutating method receives a value snapshot of its receiver; here `prepare` loads a complete
Graph before calling `adjust`, which reads `hints` and optionally `direction`. Decoded indices use
the material's high-bit closure mask. The mask is not a bounds check: initialized counters and the
exact call sequence prove the accesses are in range. An ablation changes one source feature to
test a hypothesis; it is not a compiler optimization.

## Process report

Fresh original-material compiles reproduce zero versus six exponentials for both entries. Merely
returning a complete graph or introducing another helper does not reproduce that gap. Top-down
reduction keeps the original constructor with a simple absorption consumer, then reduces normal
preparation and layer writes. Generalizing that sequence yields the standalone fixture. Its
nonmutating `adjust` branches on zero-initialized `hints`; the branch is inactive in every tested
entry, yet retaining its source shape changes downstream optimization.

The Slang producer uses canonical field keys, partial aggregate stores, ordinary calls and a
by-value receiver snapshot. NVVM input contains typed field GEPs, a whole-Graph load in `prepare`,
a call to `adjust`, and an indexed normal store. The final optimized PTX has already removed the
calls and branch, but reloads the initialized counters, uses dynamic addresses for array stores,
then reloads absorption and executes three exponentials. NVRTC removes the constant exponentials.
This is valid input and missed optimization, not evidence of malformed AST/IR or a wrong runtime
result. The exact internal libNVVM pass responsible remains unidentified.

The final unmasked variant still exhibits the difference; the branchless variant removes it.
Earlier larger variants behaved differently when the mask was removed. Layer removal, scalar
absorption and payload-only return also retain the difference in exploratory variants. Consequently
the full graph return, vector absorption, a particular late layer store, and the mask are not
individually necessary explanations. Nor are surviving calls, branches or initialization loops:
none remains in the successful small reproducer's optimized code. Combined source/layout changes
can erase the difference, so this is an explanatory reduction, not a proof of a unique minimum.

Existing shared machinery already owns related facts. `canAddressesPotentiallyAlias` distinguishes
different field keys; `canInstHaveSideEffectAtAddress` conservatively handles whole-object pointer
arguments to calls. `tryRemoveRedundantLoad` forwards matching stores within one block, while
`isPromotableVar` intentionally excludes partial field/array stores. Load narrowing and argument
specialization have separate stability/root contracts. No new alias helper, fallback, pointer
annotation or compiler special case is justified by this research alone. A subsequent bounded
experiment should locate the missed forwarding/inlining boundary using these existing contracts.

The first runtime attempt requested named entries, but render-test's compute path still selects
`computeMain`. All eighteen attempts failed before GPU execution. Their logs and source snapshots
are retained; six wrappers now select the exact `computeMain` body inspected in standalone PTX.
The failed wrapper bytes were recovered exactly, but their first shared-header whitespace snapshot
was not preserved. Its semantic reconstruction and raw dumps are retained; final accepted headers
and wrappers have exact snapshots and hashes.
No harness change or retry hides that failure. Array checks make those results observable, while
the runtime-dependent absorption control prevents a constant-output implementation from satisfying
all tests. Correct constant-folding may still eliminate construction completely.

[Evidence266](research-evidence.slice-266.json) records 18/18 exact-entry GPU passes, zero ignored,
and 12/12 final O3 compiles/assemblies. Both original-material entries also compile/assemble in both
O3 backends. All three constant/control fixture pairs have independent outputs; constant exponential
counts are 0/3 for masked/unmasked and 0/0 for branchless. Positive-control counts are 2/6, 2/6 and 2/2.
These are static whole-module counts, not GPU timing. Independent review rehashed all seven source
inputs and twelve final PTX/cubin pairs and audited the exact pass inventory and source bounds.
The lead additionally verified all 22 dependency pins, original-material artifacts and 32 unchanged
recorded tool/runtime identities. The only old provenance mismatch is the expected results-reporter
change from 265. Loaded-library traces confirm the accepted compiler and selected CUDA libraries.

The helper inventory contains research-shader constructors, normal preparation, layer construction
and output checks only; none is a new compiler helper or fallback. Formatting was checked with the
repository script; Slang attributes were restored to manual Allman layout after the C++ formatter
mangled their presentation, then all final fixture checks were rerun against the final bytes.

The accepted262 compiler remains authoritative. This slice adds research coverage, not new support,
material runtime assessment or a GPU performance claim. The general loop remains stopped, and no
compiler optimization or push is authorized by this completed experiment.

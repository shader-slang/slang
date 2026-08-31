# Slice 152 report: canonical specialized-function identity

## 1. Motivation

Consider a helper called once with a dynamically typed interface value and once with a concrete
value:

```slang
float processStatic(ICompute op)
{
    return op.defaultValue();
}

ICompute dynamicOp = condition ? AddOp(10.0) : MulOp(3.0);
float a = processStatic(dynamicOp);
float b = processStatic(MulOp(5.0));
```

Typeflow correctly needs two implementations after specialization. The dynamic call uses a tagged
union representation for `ICompute`; the concrete call can use `MulOp` directly. Before this slice,
the concrete clone copied the source helper's `IRExportDecoration`. Both live definitions therefore
claimed the same external symbol even though their signatures differed, and direct NVVM stopped
before provider mutation.

A second shape appeared in variadic helpers. `tupleSize2<>` and `tupleSize2<int, int>` are distinct
generic specializations, but the specialized-linkage digest saw an empty spelling for both
canonical `IRTypePack` arguments. Each received the SHA-1 empty-input suffix `da39a3...`, again
producing two live definitions for one symbol.

The frozen corpus contained six function-identity rows and discovery contained two. Together they
made this the largest shared exact root-cause family after Slice 151.

## 2. Proposed solution

Repair identity at both canonical specialization producers:

1. Immediately remove import/export linkage from the function cloned by
   `lowerSpecializeExistentialsInFunc`. The base source function retains its external linkage; the
   context-specific clone is an internal implementation variant.
2. Add recursive `IRTypePack` spelling to the existing `getTypeNameHint` source of truth. The
   existing `specializeLinkageDecoration` digest then distinguishes pack arity, order, and element
   types without a second mangling path.
3. Keep direct NVVM's duplicate-name validation strict and make its diagnostic include the exact
   colliding symbol. The emitter does not suffix or otherwise repair malformed external linkage.

No provider callback, LLVM text manipulation, or ABI revision is needed.

## 3. Change summary

- `source/slang/slang-ir-typeflow-specialize.cpp`
  - removes linkage decorations from exact existential-context clones while preserving name hints.
- `source/slang/slang-ir-util.cpp`
  - spells canonical `IRTypePack` values as ordered recursive `type_pack<...>` names.
- `source/slang/slang-emit-nvvm.cpp`
  - reports the exact duplicate function symbol when the upstream identity invariant is violated.
- `issue-nvvm-backend/summarize-compute-census.py` and
  `issue-nvvm-backend/summarize-compute-discovery.py`
  - classify the precise diagnostic under the existing function-identity family.
- Seven repository shaders
  - add stable direct-NVVM O0/O3 differential lanes for the newly correct identities.
- Slice 152 frozen/discovery TSV, cluster JSON, measurement manifest, plan, report, design, and
  capability ledger preserve the separate evidence and producer rationale.

## 4. Concepts and vocabulary

- **External linkage:** an import/export decoration that assigns a source-level symbol identity to
  an IR definition. Two live definitions cannot claim one external symbol merely because they
  originated from one source declaration.
- **Existential-context clone:** the internal function made for one concrete typeflow context while
  a wider tagged-union implementation remains reachable for another call.
- **Specialized-linkage digest:** the existing SHA-1 suffix computed from canonical specialization
  argument spellings and appended to a generic function's linkage name.
- **Name hint:** readable, non-semantic metadata. It can remain on an internal clone without making
  that clone externally linked.

## 5. Process report

### Reachable-function collection was not duplicating one function

The initial E52017 diagnostic said only `function name`, so the first audit checked the NVVM
consumer. `_visitNVVMFunction` maintains `functionSet`, `activeFunctions`, and
`completedFunctions`; it records one `IRFunc*` only once. The collision therefore had to involve
distinct functions.

The diagnostic was tightened to print the exact name. A post-`collectMetadata` dump of
`static-method-dispatch.slang` showed:

```text
[export("...processStatic...")]
func %processStatic  : Func(Float, %Tuple)

[export("...processStatic...")]
func %processStatic1 : Func(Float, %MulOp)
```

This is valid function specialization with invalid copied linkage. It is not a valid alternate
input spelling for the NVVM emitter to accept.

### Typeflow owns the copied-linkage correction

`lowerSpecializeExistentialsInFunc` calls `cloneInst` on the base function, transfers propagation,
call-site, and return information to the clone, then later lets `specializeFunc` change its
effective signature. `cloneInst` deliberately copies all decorations and children, including the
base export. The base remains live because another call still needs its wider representation.

The clone is therefore analogous to torn-off clones in function-call specialization and
coexisting buffer-element specializations, where existing code removes linkage. Calling the shared
`removeLinkageDecorations` immediately after cloning establishes the invariant at construction:
the base remains the sole external definition and the context-specific clone is internal. Its name
hint remains available to diagnostics and text emitters.

Removing this one call restores the exact duplicate-symbol failure for five workloads. The tests
prove this layer owns the rule because each contains both the wider and concrete call context that
causes this producer to keep both definitions.

### Canonical type-pack spelling owns the generic collision

After the typeflow fix, `size-of-tuple.slang` and
`variadic-pack-query-pack-conformance.slang` still failed. Their dump showed two different
signatures with the same already-specialized export suffix. The suffix producer is
`specializeLinkageDecoration` in `slang-ir-clone.cpp`. It already delegates argument identity to
`getTypeNameHint` and hashes the resulting bytes.

`IRTypePack` is a canonical hoistable type whose operands are precisely its flattened ordered
elements. `getTypeNameHint` had no case for it, so the default branch emitted nothing when the pack
had no name decoration. Empty and non-empty packs consequently both hashed the empty byte stream.

The retained change adds `type_pack<...>` at that shared spelling boundary and recursively uses the
same helper for each operand. It does not compare arbitrary graphs, rediscover generic context, or
rebuild syntax from semantic data. Removing the case returns both variadic workloads to the exact
duplicate name with suffix `da39a3...`.

### Rejected alternatives

- Suffixing collisions in `_collectNVVMFunctionNames` was rejected because it would silently change
  external linkage semantics and hide malformed upstream IR.
- Using module order or `IRFunc*` identity in names was rejected because neither describes the
  semantic specialization and neither is stable across compilation.
- Adding a pack-only encoder inside `specializeLinkageDecoration` was rejected because
  `getTypeNameHint` is already the shared source of canonical IR argument spelling.
- Removing linkage from every specialized generic was rejected because the measured generic-pack
  definitions intentionally retain distinct specialized source linkage once their canonical
  arguments are represented correctly.

### Coverage and cascades remain explicit

Frozen corpus v1 remains exactly 452 workloads with 427 healthy MVP references:

| Frozen corpus-v1 metric | Slice 151 | Slice 152 |
|---|---:|---:|
| Direct O0 correct | 372/427 | 377/427 (88.3%) |
| Direct O3 correct | 376/427 | 381/427 (89.2%) |
| Correct in both modes | 372/427 | 377/427 (88.3%) |
| Newly correct in both modes | - | 5 |
| Old-correct regressions | - | 0 |
| Selected NVVM unit prefix | 427/427 | 427/427 |

The newly correct frozen identities are:

- `hlsl-intrinsic/size-of/size-of-tuple.slang#cuda-1`;
- `language-feature/dynamic-dispatch/buffer-struct-with-interface-field.slang#cuda-1`;
- `language-feature/dynamic-dispatch/static-method-dispatch.slang#cuda-1`;
- `language-feature/dynamic-dispatch/this-return-chained.slang#cuda-1`; and
- `language-feature/if-let/if-let-1.slang#cuda-1`.

Across all 452 rows, native NVRTC remains 449 correct and three infrastructure results. Direct O0
is 390 correct, 46 preflight, eight runtime mismatch, and eight provider; direct O3 is 395 correct,
46 preflight, eight runtime mismatch, and three provider. The sixth former function-identity row,
`generic-interface-nested.slang`, advances to provider operation
`by-value aggregate field pointer` with result `-2147024809`. It is not promoted or counted as
supported.

Discovery remains 82 workloads with 72 healthy native references:

| Discovery metric | Slice 151 | Slice 152 |
|---|---:|---:|
| Direct O0 correct | 52/72 | 54/72 (75.0%) |
| Direct O3 correct | 52/72 | 54/72 (75.0%) |
| Correct in both modes | 52/72 | 54/72 (75.0%) |
| Newly correct in both modes | - | 2 |
| Old-correct regressions | - | 0 |

The newly correct discovery identities are
`language-feature/dynamic-dispatch/array-of-interfaces-interproc.slang#discovery-1` and
`language-feature/generics/variadic-pack-query-pack-conformance.slang#discovery-1`. Each direct
mode has 54 correct, 19 preflight, one provider, seven infrastructure, and one runtime mismatch.
The function-identity cluster is eliminated from both corpora; their denominators remain separate
and no corpus v2 is proposed.

### Exploratory performance and architecture evidence

All twelve discovery measurement gates compile and assemble through CUDA 12.9 `ptxas` at direct
O3 for SM70, SM80, and SM90. The new gates measure:

| Workload/configuration | Median compile | PTX size | Cubin size |
|---|---:|---:|---:|
| Existential call graph, NVRTC O3 | 370.6 ms | 8946 B | 13792 B |
| Existential call graph, direct O0 SM70 | 252.0 ms | 60001 B | 22888 B |
| Existential call graph, direct O3 SM70 | 261.8 ms | 1007 B | 3048 B |
| Variadic pack, NVRTC O3 | 351.1 ms | 8585 B | 13664 B |
| Variadic pack, direct O0 SM70 | 236.8 ms | 47577 B | 14952 B |
| Variadic pack, direct O3 SM70 | 239.5 ms | 646 B | 2792 B |

The large O0 output is an exploratory optimization-quality signal. Runtime correctness comes from
the differential corpus and permanent compare-compute lanes; the compile timings are not presented
as a controlled benchmark.

### Self-review inventory

- New special case: `IRTypePack` in `getTypeNameHint`. It survives because the canonical type's own
  operands are the existing semantic source of truth, two tests fail without it, and every caller
  benefits from the same spelling.
- New producer action: remove linkage from the exact existential-context clone. It survives because
  the copied export is accidental, the base remains the source symbol, and established
  specialization passes enforce the same invariant.
- Diagnostic widening: exact duplicate symbol text. It survives because it reports malformed
  canonical linked IR before provider mutation and does not admit or repair the shape.
- Census mappings: precise and legacy function-name diagnostics map to one producer family. They
  affect evidence classification only and do not alter compiler acceptance.

No syntax reconstruction, fixture-name check, custom equivalence relation, compatibility codegen
fallback, downstream IR text patch, provider callback, or ABI revision was added.

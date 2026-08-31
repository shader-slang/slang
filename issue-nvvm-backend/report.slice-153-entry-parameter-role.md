# Slice 153 report: entry parameter role classification

## 1. Motivation

Consider an interface value selected across control flow:

```slang
IProcessor<float> op;
if (condition)
    op = FloatDoubler();
else
    op = FloatNegator();
output[0] = op.process(5.0);
```

Existential lowering represents `op` as a tagged tuple. Each branch sends one tuple to the merge
block, whose `%op : Tuple` block parameter is an `IRParam`. The first operation after the merge
extracts the runtime tag with `get_field(%op, %value0_)`.

Before this slice, direct NVVM treated that block parameter as a physical CUDA kernel parameter
because the current function was the entry point and the value's opcode was `IRParam`. It called
the provider's struct-field-pointer operation on a first-class LLVM struct. The typed provider
correctly rejected the non-pointer value with `SLANG_E_INVALID_ARG`. Three frozen-v1 dynamic-
dispatch workloads shared this exact first failure.

## 2. Proposed solution

Use the canonical IR ownership relation to distinguish the two roles. A function parameter is an
`IRParam` whose parent is the function's first block. An `IRParam` in any later block is an SSA phi
value. Direct NVVM now requires first-block ownership before selecting the pointer-backed CUDA
`byval` aggregate path.

Actual aggregate launch parameters continue to use their existing generic pointer representation,
`byval` attributes, struct GEP, and invariant load. Merge-block tuples use the existing generic
aggregate extraction operation. No new representation, provider callback, or compatibility path is
needed.

## 3. Change summary

- `source/slang/slang-emit-nvvm.cpp`
  - restricts pointer-backed entry aggregate field extraction to first-block parameters and explains
    the existential merge example.
- Three dynamic-dispatch shaders
  - add direct-NVVM O0/O3 differential lanes after real-provider correctness was proven.
- Slice 153 frozen/discovery TSV and cluster JSON artifacts
  - preserve separate denominators, classifications, and remaining Pareto data.
- Slice 153 plan, measurement manifest, design document, capability ledger, and this report
  - record the canonical producer/consumer trace and validation evidence.

## 4. Concepts and vocabulary

- **Function parameter:** an `IRParam` owned by the function's first block; it participates in the
  physical function ABI.
- **Block parameter:** an `IRParam` owned by a later block; it represents an SSA phi value supplied
  by predecessor branches.
- **Pointer-backed `byval` parameter:** LLVM's CUDA launch representation for a source aggregate
  passed by value. The LLVM function argument is a pointer annotated with the aggregate pointee
  type and alignment.
- **First-class aggregate:** an LLVM struct or array value transported directly through SSA and
  accessed with aggregate extract/insert operations.

## 5. Process report

### The provider failure exposed a role mismatch, not a missing operation

The Slice 152 census classified three rows under `provider-aggregate-field-pointer`. A final linked
IR dump of `generic-interface-dynamic-param.slang` showed that the entry function had no remaining
launch parameters. Its failed field extraction instead used `%floatOp : Tuple`, a parameter of the
merge block joining the two interface constructions.

`NVVMTypeLoweringContext` already has two correct representations. A scalar struct used as
`EntryPointParameter` lowers to a generic pointer and receives physical `byval` attributes. An
ordinary `Value` aggregate lowers to an LLVM struct. The provider already has strict typed
operations for both: `emitStructFieldPointer` accepts a pointer to a struct, while
`emitAggregateElementExtract` accepts a first-class struct. The failure occurred because emission
selected the former for the latter.

Adding a provider callback or making `emitStructFieldPointer` accept aggregates was rejected. That
would merge two distinct LLVM operations and weaken the typed boundary without representing a new
canonical Slang operation.

### Parent block is the canonical discriminator

Slang deliberately uses `IRParam` for both roles. The representation records the distinction in
the instruction's parent: parameters of `func->getFirstBlock()` are function inputs, while later-
block parameters are phi definitions. Existing differentiation, SSA allocation, and SPIR-V
legalization code uses this same relation.

The retained change adds that exact ownership requirement to the `kIROp_FieldExtract` classification.
It does not walk operand graphs, infer syntax, compare structural substitutes, or identify any
fixture. Removing the parent check reproduces the provider failure in all three motivating tests.
The test layer owns the behavior because these workloads deliberately keep existential tuples live
across branch merges and extract their tags inside the entry function.

### Physical aggregate launch behavior remains independently protected

The selected NVVM unit prefix includes `nvvmIRBuilderPreservesByValueParameterContracts` and the
established conventional aggregate entry-point tests. All 427 selected tests pass after the change.
The six newly promoted O0/O3 lanes also pass through the real provider. Thus the role correction
changes only the invalid merge-phi classification and preserves the real first-block `byval` path.

### Coverage and remaining Pareto

Frozen corpus v1 remains exactly 452 workloads with 427 healthy MVP references:

| Frozen corpus-v1 metric | Slice 152 | Slice 153 |
|---|---:|---:|
| Direct O0 correct | 377/427 | 380/427 (89.0%) |
| Direct O3 correct | 381/427 | 384/427 (89.9%) |
| Correct in both modes | 377/427 | 380/427 (89.0%) |
| Newly correct in both modes | - | 3 |
| Old-correct regressions | - | 0 |
| Selected NVVM unit prefix | 427/427 | 427/427 |

The newly correct frozen rows are:

- `language-feature/dynamic-dispatch/generic-interface-dynamic-param.slang#cuda-1`;
- `language-feature/dynamic-dispatch/generic-interface-multi-conform.slang#cuda-1`; and
- `language-feature/dynamic-dispatch/generic-interface-nested.slang#cuda-1`.

Across all 452 rows, native NVRTC remains 449 correct and three infrastructure results. Direct O0
is 393 correct, 46 preflight, eight runtime mismatch, and five provider; direct O3 is 398 correct,
46 preflight, and eight runtime mismatch. The provider aggregate-field-pointer cluster is
eliminated. The leading remaining O0 families include nine helper-ABI type-contract rows, nine
preflight-other rows, seven aggregate-pointer/layout rows, six wave-reconvergence generic-asm rows,
and five unoptimized-half provider rows.

Discovery remains exactly 82 workloads with 72 healthy native references:

| Discovery metric | Slice 152 | Slice 153 |
|---|---:|---:|
| Direct O0 correct | 54/72 | 54/72 (75.0%) |
| Direct O3 correct | 54/72 | 54/72 (75.0%) |
| Correct in both modes | 54/72 | 54/72 (75.0%) |
| Newly correct in both modes | - | 0 |
| Old-correct regressions | - | 0 |

Each discovery direct mode remains 54 correct, 19 preflight, one provider, seven infrastructure,
and one runtime mismatch. This is expected: its remaining struct-field and sequential-pointer rows
come from distinct pointer producers, not entry-function block-parameter field extraction. The two
denominators remain separate and no corpus v2 is proposed.

### Exploratory architecture and output evidence

All twelve established discovery measurement gates compile and assemble through CUDA 12.9 `ptxas`
at direct O3 for SM70, SM80, and SM90. The existential-specialization gate, which continues to
exercise aggregate transport across a larger dynamic call graph, measures 271.9 ms and 1007-byte
PTX at direct O3 SM70, versus 365.7 ms and 8946-byte PTX through NVRTC O3. Timings remain
uncontrolled exploratory measurements rather than benchmark claims.

### Self-review inventory

- Widening: one parent-block conjunct in the existing pointer-backed entry-parameter classifier.
  It survives because parent ownership is the canonical role representation, three tests fail
  without it, and existing first-block ABI tests prove the unaffected branch.
- New regression directives: six direct lanes across three existing deterministic shaders. They
  survive because each covers a distinct generic-interface combination sharing the same canonical
  merge-parameter invariant.

No helper/fallback, syntax reconstruction, custom equivalence relation, fixture-name check,
downstream text patch, provider callback, or ABI revision was added.

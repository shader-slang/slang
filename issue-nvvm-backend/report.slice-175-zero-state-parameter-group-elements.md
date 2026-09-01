# Slice 175: Zero-state parameter-group elements

## Motivation

The final healthy discovery failure combines dynamic dispatch with a parameter block whose
element recursively has no data state:

```slang
interface IFoo
{
    associatedtype T : IFoo;
    T getT();
    void doSomething();
}

struct A : IFoo
{
    typealias T = B;
    T getT() { return {}; }
    void doSomething() { outputBuffer[0] = 1; }
}

struct B : IFoo
{
    A a;
    typealias T = A;
    T getT() { return {}; }
    void doSomething() { outputBuffer[0] = 1; }
}

ParameterBlock<B> gB;
```

`A` has zero fields and `B` contains only `A`, so both have zero bytes of state. CUDA global-
parameter collection nevertheless retains `gB` as an eight-byte typed handle in the synthesized
`GlobalParams` record. Direct NVVM rejected the canonical pointee while classifying that launch
field because its ordinary aggregate-storage grammar deliberately requires a nonempty struct.

## Proposed solution

Give parameter-group element storage one explicit recursive role that permits finite zero-state
structs. Enter that role only through an exact `ParameterBlock` or `ConstantBuffer`, then carry it
through type lowering, provider/CUDA layout proof, and retained declaration collection. Preserve
the exact nested struct types, including a zero-field LLVM struct, behind the typed global pointer.

Do not widen ordinary aggregate storage, copyable values, helper values, or helper signatures.
Keep active-type cycle rejection and all existing exact parameter-group opcode checks. LLVM 14 and
the revision-32 provider can already create a zero-field struct and typed pointer, so this is
compiler-side classification/legalization and does not require a provider ABI change.

## Change summary

- A parameter-group-element storage classifier reuses the recursive aggregate algebra with a
  role-local zero-state permission; ordinary callers retain the nonempty rule.
- Parameter-group recognition, storage-value proof, and type lowering preserve exact empty and
  transitively empty structs and arrays.
- Aggregate layout validation and selected declaration retention carry the same explicit role.
- `bugs/type-legalize-bug-1.slang` gains permanent O0/O3 direct-NVVM differential lanes.
- Frozen/discovery TSV and Pareto JSON, the representative measurement manifest, design notes,
  capability ledger, plan, and this report retain the evidence.

## Concepts and vocabulary

**Zero-state struct** is an `IRStructType` with no data fields, or whose fields recursively
contribute zero bytes. It still has a canonical type identity even though it contributes no
storage bytes.

**Parameter-group element role** is the pointee storage role established by an exact
`ParameterBlock<T>` or `ConstantBuffer<T>`. The externally visible handle remains pointer-sized
regardless of whether `T` has data state.

**Selected declaration closure** is the set of module-scope struct declarations required to spell
the types of selected functions, locals, resources, and launch parameters after linking.

## Process report

`collectGlobalUniformParameters` in `slang-ir-collect-global-uniforms.cpp` builds the conventional
`GlobalParams` struct and retains `gB` as `ParameterBlock<B>`. Its CUDA layout is size 24,
alignment eight: the output structured buffer occupies the first 16 bytes and the `gB` handle is
at offset 16. Dynamic-dispatch optimization folds the relevant interface tags and removes the
eventual `gB` load, but it does not rewrite the source launch contract. The final linked module
therefore still contains the typed handle and the `B -> A -> empty` declaration graph.

`asNVVMSupportedParameterGroupType` previously asked the ordinary recursive aggregate-storage
classifier to prove `B`. That classifier reached `A`, saw no fields, and returned false. The
retained classifier now takes an explicit `allowZeroStateStructs` role. The existing ordinary
entry point passes false. The new `isNVVMSupportedParameterGroupElementStorageType` entry point
passes true, and nested parameter groups establish the same role for their own pointees. Arrays
propagate the role while still requiring a positive finite element count and canonical stride;
active-type tracking continues to reject recursive type cycles.

The exact input shape is intentionally valid at this layer. The producer is
`collectGlobalUniformParameters`, and the semantic source of truth is the retained
`IRParameterGroupType` plus its element `IRStructType`; no syntax is reconstructed. The launch ABI
requires a typed pointer-sized handle even when downstream optimization removes all loads. Fixing
the producer to erase the field would change reflection and host binding, while replacing the
pointee with bytes would discard canonical type identity. Parameter-group classification is
therefore the correct ownership boundary.

`NVVMTypeLoweringContext::lowerType` admits the wider grammar only for
`NVVMTypeUse::ParameterGroupStorage`. `_lowerParameterGroupType` recursively lowers `B` and `A`,
and the existing provider struct constructor creates the exact zero-field LLVM type for `A`.
Ordinary storage and helper/copyable entry points still reject an empty root. The storage-value
predicate also accepts an empty struct only under the parameter-group role, so a future canonical
load can be classified without implying that an arbitrary empty helper value is supported.

After this first fix, the workload advanced to `parameter-group storage layout`, and then to a
module-scope `struct` diagnostic. `_getNVVMAggregateStorageLayout` had reapplied the ordinary
nonempty grammar and rejected the zero-size child. Its explicit role now selects the same
parameter-group element classifier, permits exactly size zero with positive alignment, and still
compares complete provider size/alignment and every retained offset/stride against CUDA layout.
This proves rather than assumes that the empty child contributes no bytes.

The later module-scope diagnostic came from `_addNVVMReachableStructTypes`. Conventional-global
validation had already removed the parameter-group wrapper before passing `B`, so the closure
collector lost the provenance that allowed `A`. It now receives and recursively carries the
explicit parameter-group role, retaining `B` and `A` as selected declarations. Other roots use the
default false value and remain subject to helper/resource/ordinary nonempty classifiers. This is
the same representation cascade, not a fallback for an unrelated `struct` diagnostic.

The self-review inventory contains four retained changes. The role-aware recursive classifier is
required because removing it restores the initial `ParameterBlock<B>` field failure. The storage-
value change is the corresponding exact value-representation proof and does not widen helper
signatures. The layout role is required because removing it restores the next canonical layout
failure. The declaration-closure role is required because removing it restores the retained
module-scope `struct` rejection. All four consume the same producer-owned parameter-group role.
No fixture-name check, syntax reconstruction, compatibility fallback, arbitrary operand walk,
malformed upstream IR patch, diagnostic weakening, or provider callback was added.

Frozen corpus v1 remains exactly 452 workloads and 427 healthy MVP references. Healthy
correctness remains 413/413/413 O0/O3/both with zero semantic classification, shape, producer,
diagnostic, cluster, or evidence change. All-row direct totals remain 427 correct, four runtime
mismatches, and 21 preflight failures in each mode. Discovery remains exactly 82 workloads and 72
healthy references and advances from 71/71/71 to 72/72/72, with only
`bugs/type-legalize-bug-1` gained and no loss.

The selected regression prefix passes 433/433 and the permanent `nvvm` category passes 80/80.
The representative gate compiles and assembles through CUDA 12.9 for native NVRTC, direct O0
SM70, and direct O3 SM70/SM80/SM90. At SM70, direct O3 PTX is 477 bytes versus 8,416 bytes native,
and median standalone compile time is 252.0 ms versus 382.0 ms. These remain exploratory
measurements rather than a controlled benchmark.

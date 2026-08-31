# Slice 160: Resource-aggregate parameter transport

## Motivation

Specialization already turns interface-typed shader parameters into finite concrete structs, but
direct NVVM still rejected those structs at the two compute parameter boundaries. Consider the raw
entry-point case from `interface-func-param-in-struct.slang`:

```slang
struct Params
{
    StructuredBuffer<IInterface> obj;
};

void compute(uint tid, Params p)
{
    gOutputBuffer[tid] = p.obj[0].eval();
}

void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID, uniform Params params)
{
    compute(dispatchThreadID.x, params);
}
```

After shader-object specialization, `Params` is a canonical finite struct whose resource leaf has
the backend's established raw-view representation. The entry parameter must nevertheless obey the
NVPTX kernel ABI, where an aggregate is a generic pointer carrying `byval`. Mapping that physical
pointer directly to the semantic `IRParam` made field addressing work but passed a pointer to
`compute`, whose signature expects the first-class struct value.

The conventional path had a related but physically different gap. Consider
`copy-elision-this-2.slang`:

```slang
struct Data { int val; }
uniform Data globalData;
RWStructuredBuffer<int> input;
RWStructuredBuffer<int> output;

int addCopyElision(Data data, int val)
{
    return data.val + val;
}
```

The parameter collector places `globalData`, `input`, and `output` in one synthesized global
parameter struct. `Data` is already a valid finite provider value, but the conventional field
classifier admitted individual scalars and resources rather than that aggregate field.

## Proposed solution

Use `asNVVMSupportedResourceStructType` as the single finite aggregate classifier at both
boundaries, while preserving their distinct physical contracts.

For raw launch parameters, retain the provider pointer as the physical ABI value, attach exact
`byval` pointee and CUDA alignment attributes, and materialize one aligned invariant whole-struct
load for ordinary semantic uses. Keep a separate parameter-to-pointer map so a direct field extract
from the entry parameter still emits typed GEP plus invariant field load. A helper call therefore
receives the first-class aggregate value, not the launch pointer.

For conventional globals, admit the finite struct as one field of the existing synthesized
constant-address-space block. Validate exact recursive CUDA/LLVM layout, add its nested types to the
reachable struct closure, and reuse keyed field addressing. This value never receives a `byval`
wrapper.

The change uses provider ABI revision 30's generic struct, pointer, parameter-attribute, GEP, and
load operations. No provider callback, textual IR patch, fixture-name check, compatibility
fallback, or source reconstruction is needed.

## Change summary

- NVVM type lowering accepts the established finite resource-struct algebra for entry parameters
  and conventional global fields.
- NVVM validation requires exact recursive layout compatibility at both boundaries.
- Raw entry aggregates retain separate physical-pointer and semantic-value mappings.
- Conventional aggregate fields use the existing immutable keyed storage path.
- Four frozen-v1 workloads and one discovery workload gain direct O0/O3 regression lanes.
- Frozen/discovery census and Pareto snapshots remain separate, and the representative measurement
  manifest grows from 16 to 17 standalone gates.
- The obsolete nested-aggregate preflight-negative case is removed while its genuinely invalid
  neighboring layout and matrix-operation cases remain.
- The design guide, capability ledger, ExecPlan, and this report record the invariant and evidence.

## Concepts and vocabulary

A *physical launch value* is the LLVM value required by the NVPTX kernel ABI. For an aggregate it
is a generic pointer annotated with `byval(T)` and an alignment. A *semantic aggregate value* is
the first-class LLVM struct expected by ordinary Slang IR operations and helper signatures.

A *conventional global block* is the synthesized constant-address-space struct used when ordinary
CUDA shader parameters are collected rather than written directly on the kernel signature.

The *resource-struct classifier* is the recursive, cycle-safe
`asNVVMSupportedResourceStructType` predicate. It admits a finite non-empty struct only when every
leaf already has a selected provider representation.

## Process report

The first probe admitted the specialized `Params` type in
`NVVMTypeLoweringContext::lowerType`. `_validateNVVMFunction` then accepted the entry signature,
and function declaration correctly produced a generic pointer with exact `byval` metadata. The
probe subsequently failed in the provider's generic call operation: `valueMap[param]` still named
that pointer when `_getLoweredNVVMValue` assembled the argument to `compute(Params)`.

The pointer is not malformed; it is the correct physical spelling for exactly one role. The error
was using it as the source of truth for every later role. `emitNVVMIRFromLinkedIR` now records the
original entry handle in `entryAggregatePointerMap`. For non-scalar finite resource structs it
emits one invariant load in the entry block and replaces the ordinary `valueMap` entry with the
first-class result. Generic helper calls then follow their existing exact type contract. When an
`IRFieldExtract` directly names the first-block entry parameter, the emitter deliberately consults
the physical map and emits `emitStructFieldPointer` followed by an aligned invariant load. Later
block parameters and ordinary aggregate values continue through generic aggregate extraction.

The conventional producer is `_getNVVMConventionalGlobalParams`, which identifies the synthesized
outer struct. `isNVVMSupportedConventionalGlobalFieldType` now admits a field only when the same
recursive resource-struct classifier accepts it. `validateNVVMSupportedIR` calls
`_hasNVVMCompatibleStructLayout` before provider creation and records every reachable nested
struct. `_getNVVMStructFieldAddress` recognizes the same exact field family, so existing keyed GEP
and immutable-load logic remains the only consumer path. The raw launch and conventional paths
share a value algebra, not a pointer representation.

The sixth selected workload, `generic-shader-object-cbuffer2.slang`, did not contain that shape.
Its entry type remains the `ParameterBlock<Impl<...>>` wrapper itself. That canonical producer
needs a parameter-group launch ABI, so the slice records the new first blocker and does not widen a
resource-struct rule to cover it.

The self-review inventory found no new helper or fallback. It found three deliberate conditionals:

1. Entry aggregates use `asNVVMSupportedResourceStructType` only after exact layout validation.
   The four raw/conventional frozen rows and discovery `interface-shader-param` fail without this
   widening and pass differentially with it.
2. Only non-scalar resource aggregates receive the new whole-value entry load. Existing scalar
   aggregate field access keeps its established contract; the new load exists because the selected
   resource aggregate is passed whole to an ordinary helper.
3. Direct entry-field extraction consults the physical pointer map only for an `IRParam` owned by
   the entry function's first block. This preserves Slice 153's distinction between launch ABI
   parameters and later-block SSA parameters.

The former negative fixture `Outer { Inner { uint value; }; }` is a finite, canonical aggregate,
not an invalid alternative spelling. The recursive classifier now admits it, so retaining an
assertion that it must fail before provider mutation would encode an obsolete slice boundary. The
fake builder aliases generic struct handles too aggressively to model the nested positive case.
Instead, a real LLVM 14 provider probe passes at O0 and O3 while sending the whole value to a
helper, and both PTX outputs assemble for SM70. Only that stale source was removed from the
negative array; incompatible structured-buffer layout and unsupported structured-matrix write
remain deterministic preflight failures.

Validation used the Windows-native Release build and ran outside the sandbox. The selected NVVM
prefix passes 427/427. Frozen corpus v1 remains exactly 452 workloads/427 healthy references and
improves from 386/390/386 to 390/394/390 O0/O3/both-mode correctness, with zero old-correct loss.
All-row classifications are native 449 correct/three infrastructure; direct O0 403 correct,
36 preflight, eight runtime mismatch, and five provider; direct O3 408 correct, 36 preflight, and
eight runtime mismatch.

Discovery remains exactly 82 workloads/72 healthy references and improves from 60/60/60 to
61/61/61, with zero old-correct loss. Each direct mode has 61 correct, 11 preflight, two provider,
seven infrastructure, and one runtime mismatch. All seventeen representative direct-O3 gates
assemble with CUDA 12.9 for SM70, SM80, and SM90. The new conventional aggregate/helper gate
measures 258.1 ms and 797-byte PTX at direct O3 SM70 versus 370.3 ms and 8770 bytes through NVRTC
O3; direct O0 measures 255.1 ms and emits 22133-byte PTX. These timings remain exploratory.

The repository formatter was attempted with `--modified`, but gersemi, clang-format, prettier,
and shfmt are unavailable on this machine. Manual review, `git diff --check`, JSON parsing, exact
TSV identity/count checks, and measurement completeness checks pass.

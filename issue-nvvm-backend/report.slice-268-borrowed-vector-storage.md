# Preserve native vector storage in borrowed helpers

## Motivation

A valid borrowed aggregate containing float3 fails direct NVVM at O0 and O3 with E52018,
compact parameter-group vector extraction. Consider this helper:

```slang
struct Payload
{
    float3 value;
    float sentinel;
};

[noinline]
float3 readPayload(__constref Payload payload)
{
    return payload.value;
}
```

The borrowed parameter is read-only, but its float3 field uses a native LLVM vector. The old
emitter mistakes read-only access for compact storage and asks the provider to extract fields
from that vector as if it were a compact aggregate. Float4 succeeds because it is outside this
compact conversion family. The unchanged267 compiler reproduces both failures and controls;
the new output-checked borrowed fixture passes NVRTC while both NVVM modes fail.

## Proposed solution

Preserve the storage role already selected by type lowering through the emitter's existing field
and sequential-element resolver records. Compact vector extraction requires both its existing
immutable-access constraint and an actual parameter-group storage role. Native borrowed helper
fields keep their native representation. No AST/IR producer, helper ABI or provider contract changes.

## Change summary

`source/slang/slang-emit-nvvm.cpp` adds storage-role metadata at existing roots and propagates it
through nested field and array resolution. Two CUDA fixtures exercise native borrowed vectors and
real compact buffers with independent output oracles. This report, the completed plan and structured
validation record retain acceptance and failure history; raw evidence is under
`build/nvvm-borrowed-vector268`.

## Concepts and vocabulary

`BorrowInParam` is the canonical read-only helper-reference type. `NVVMTypeUse::Value` selects
native vector layout for it. `ParameterGroupStorage` selects the device buffer representation,
including compact float3 storage. Access permission and storage layout are independent properties.

## Process report

`getExplicitlyDeclaredParamPassingMode` maps the borrow modifier to `BorrowIn`; parameter lowering
uses `getBorrowInParamType`. `asNVVMSupportedHelperReferencePointerType` recognizes the canonical
Generic/Read/DefaultBufferLayout type and helper-reference lowering selects `NVVMTypeUse::Value`.
Field-address emission uses its semantic field key and preserves the lowered base pointer. The
input is canonical and intentionally valid; changing its producer or the provider extraction
contract would be the wrong repair.

The helper inventory contains no new graph walker, recursive type classifier, equivalence helper
or syntax reconstruction. Two existing resolver records gain `isParameterGroupStorage`. Existing
parameter-group and physical-storage root branches set it consistently with type lowering; nested
fields and sequential elements propagate it. `_getNVVMCompactParameterGroupVectorPointer` consumes
that fact instead of inferring layout from immutability. The regression fails without this predicate
because the native vector is sent to aggregate extraction.

Independent review caught an unnecessary expansion in the first candidate: replacing the immutable
constraint outright would also admit mutable physical vectors, while their store path does not
establish matching compact packing support. The final candidate retains the existing immutable
constraint and adds the storage-role check. Mutable physical roots still carry their accurate role;
this slice does not extend their conversion support.

The compact fixture's first independent arithmetic oracle incorrectly wrote 1330 for
`10 + 10*11 + 100*12`. Its source and failed log remain recorded; the corrected 1320 oracle was
qualified on the unchanged baseline before compiler editing. This was a test-construction error,
not a compiler regression. Candidate attempts and subsequent corrections remain separate evidence.

The two fixtures pass all six GPU cells and six standalone compile/assembly cells. Native LLVM
loads use `<3 x float>` and `<3 x i32>`; constant-buffer loads use `[3 x float]` extraction.
Neighbours have 42 passes and 9 platform skips, including the original Graph constref experiment,
physical borrowed arrays and negative boolean-lane references. Two additional physical-reference
compile/assembly cells pass; retained IR confirms the physical borrowed receiver shape.

[Validation268](runtime-validation.slice-268.json) preserves all 1,356 frozen and 357 discovery
outcomes, 567 input hashes, 39 known gaps and 18 resolved histories. Units retain 1,086 pass/13 skip and
semantics 1,170 pass/78 skip with exact identities and statuses. Runtime 4, material 6, toolkit 18 and all
four runner contract suites pass. The initial nonexistent build-target and missing formatter-PATH
attempts are recorded; neither changed the qualified source. The first broader candidate was built
but excluded from qualification; its source/binary identity remains available. Final candidate2
source, compiler and fixture hashes match every focused and full gate.

The qualified compiler is built from 0aff56e26 plus patch
`dfb3b91cfd3d36af94c884a5ca08c5c087d5c9576d39d11b4868dac953140883`, version
`2026.18.3-284-g0aff56e26`; loaded library SHA256 is
`07881e5f945ee7028e1bc913ec127405422cd014ddc624b13a4d308f8670b822`. Provider ABI42 is unchanged.
No timing or GPU performance claim is made. The next authorized slice repairs enumeration and broadens the corpus;
the general development loop remains stopped.

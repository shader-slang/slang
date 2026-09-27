# Preserve padding in nested NVVM record stores

Status: accepted full validation; independently reviewed with a reused reviewer. Development loop active; local commits only.

## Motivation

Nested FP8/BF16 records were an explicit boundary of slice270. A three-level record containing FP8,
scalar BF16, BF2 and integer guards fails preflight at both NVVM optimization levels. Qualifying that
shape exposed a second issue: whole-record replacement passes NVRTC/O0 but corrupts narrow fields
at NVVM O3. The same problem reproduces with already-supported integer fields on accepted277.

A reduced example makes the padding requirement visible:

```slang
struct Child { uint16_t first; uint last; }
struct Outer { uint16_t prefix; Child child; }
[noinline]
Outer replace(inout Outer destination, Outer source)
{
    let previous = destination;
    destination = source;
    return previous;
}
```

CUDA layout places the two uint16 fields at offsets0 and4. Installed libNVVM12.9 turns the valid
whole-record LLVM store into a two-lane uint16 store at offsets0 and2, overwriting padding and leaving
`child.first` stale. The original FP8 fixture exposes the same mistake at two nested boundaries.

## Proposed solution

Keep canonical record types, field keys and layout. Extend only the dedicated internal substandard
record proof to finite nonempty nested records with qualified integer/FP8/scalarBF16/BF2 leaves, and
preserve local-storage permission through derived field addresses. Other storage and signature roles
retain their gates.

At the LLVM provider's physical store boundary, split a struct containing an immediate struct field
into canonical field stores, recursively. Preserve flat struct, array, scalar and vector stores. Use
LLVM DataLayout offsets and the actual parent alignment guarantee for each field. This is explicitly
a workaround for valid LLVM miscompiled by the installed downstream compiler; it does not repair or
reconstruct a malformed Slang representation.

## Change summary

The NVVM type classifier and local-field resolver admit the bounded nested domain. The LLVM provider
preserves nested store boundaries. Native tests exercise value/local/negative/layout roles and provider
alignment/store shapes; GPU fixtures exercise nested substandard and integer records with independent
integer oracles. The record contract documents qualified scope. Main corpus inventories stay unchanged.

The full1740-cell corpus comparison preserves every accepted277 outcome and all576 input hashes.
Full units preserve1103 prior identities plus six new passes (1096pass/13skip). Semantics preserve1170pass/78skip. Runtime4, toolkit18 and all four runner-contract suites pass.
The [compact validation record](runtime-validation.slice-279.json) retains exact outcomes and failed
attempts. Compiler9e013b2c/version295-g0043e8d17 and provider a861b242/ABI42 were qualified from
source0043e8d17 plus patch62ae6473; no performance claim. Raw failed candidates, LLVM/PTX, direct vendor
replays and recovery layout live under `build/nvvm-nested-records279`.

## Concepts and vocabulary

A physical record is the LLVM aggregate used for both register transport and local memory in this
domain. A padding gap belongs to that canonical layout even though no source field names it.
`commonAlignment` derives the alignment still guaranteed after adding a field offset to a pointer;
a field's ABI alignment alone cannot justify a stronger pointer guarantee. Value, local Storage,
resource storage and exported signatures remain separate admission roles.

## Process report

`visitAggTypeDecl` creates canonical IRStructType fields. `_lowerInfoFromFuncParameters` gives mutable
parameters OutParam/BorrowInOut wrappers; `extractField`/`getFieldKey` and `emitFieldAddress` preserve
canonical field keys. These are intended nested types. The dedicated classifier recursively checks
allowed leaves; the field-address resolver propagates an already-proven local-storage permission.
Widening the general device-capable helper predicate would grant unrelated roles and was rejected.

Candidate1 compiles the new domain but fails the full-encoding GPU oracle at O3. Field and stage
probes identify only the destination after whole-record replacement: both FP8 fields and a uint16
guard fail at input0. O0/O3 emit byte-identical LLVM. Direct libNVVM replay reproduces wrong output;
a standalone integer LLVM reduction reproduces it without Slang or libdevice. PTX packs values from
noncontiguous field offsets into contiguous stores, explaining all observed failed fields.

Two controlled LLVM edits replace only that whole-record store. Explicit leaf stores pass, and the
narrower split retaining flat Inner/Guards aggregate stores also passes the original exhaustive oracle
at O0/O3. This isolates the required transformation. `_emitStore` already validates exact value/pointee
type, insertion point, address space and alignment. Its new helper consumes those valid physical types,
uses LLVM's authoritative field offsets and retains existing flat operations. No source-feature test,
FP8-specific offset, parallel type model or stronger invented alignment belongs here. Arrays are kept
opaque to avoid unbounded unrolling; arbitrary array-nested layouts are not newly qualified.

The first new native positives exceeded the fake builder's singleton struct model; a minimal loader
uses the real LLVM builder and fake libNVVM so assertions inspect actual serialized assembly without
pretending fake PTX proves execution. Separate layout/negative tests retain the fake provider. Initial
negative-source delimiter and dynamic-GEP assertion mistakes are retained in attempt history. A later
unit build required braces around SLANG_CHECK, and the groupshared negative needed the exact earlier
`global_var` preflight expectation. Production logic was unchanged by those test corrections. A guard
omission control fails the independent GPU oracle, proving the integer-only child assignment is observed.

Separate root and author audits agree with a reused independent reviewer who did not author this
slice. Fresh-context delegation remains unavailable at the thread limit; no fresh-context review is
claimed. Focused GPU18/18 passes, including six complete exhaustive regression buffers. Full277
comparison preserves all1740 cells, all576 inputs and22 dependency pins; native additions are reported
separately. Last full checkpoint is279, targeted233, implementations since full0.

Material6 compiles and assembles. NVRTC and NVVM O3 PTX/cubins remain byte-identical277. Both O0
artifacts change, while every parsed resource record remains equal. Independent review finds the
same159/176 function inventories, six changed functions per module, and expected canonical field-store
expansion: field initialization replaces a336-byte zeroing loop, and field copies replace bulk
surfaceshader copies. Local depot sizes and reviewed bool-lane/address mappings remain consistent.
The original review-required artifact comparison is retained with an explicit acceptance decision.
This is bounded code-generation review, not formal equivalence or material GPU validation; material
runtime still lacks bindings/input/output contracts. Arrays containing padded records are the next
separate qualification gate; this slice does not extend their admission or guarantee.

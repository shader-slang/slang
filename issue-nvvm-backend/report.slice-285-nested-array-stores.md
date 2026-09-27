# Preserve padding in whole arrays of nested records

## Motivation

Already-supported integer array copies lose fields at NVVM O3. For example:

```slang
struct Child { uint16_t first; uint last; };
struct Cell { uint16_t first; Child child; };
typedef Cell Payload[3];
[noinline]
void assignOut(out Payload destination, Payload source)
{
    destination = source;
}
```

Each Cell has semantic offsets 0/4/8 and a 12-byte stride. Valid whole stores reach libNVVM, which can
combine the first two 16-bit fields at offsets 0/2, losing padding. Accepted279 splits direct nested
struct stores, but arrays and wrappers without immediate struct fields retain whole stores.

## Proposed solution

Preserve the canonical array value and whole store. At the provider's terminal store, detect a nested
struct boundary hidden inside an array and supply the truthful conservative alignment guarantee of
one byte. Array length does not expand provider instructions. Direct-struct splitting, allocation/load
alignment, authored LLVM call signatures, type admission and ABI42 remain unchanged.

## Change summary

- `slang-llvm-nvvm.cpp`: canonical aggregate-type predicate and terminal-store alignment selection.
- `unit-test-nvvm-builder.cpp`: one new unit identity with 39 shape/alignment cases; unchanged scalar,
  vector, flat-record array and pointer-pointee controls, plus a one-store 65,536-element boundary.
- Three `tests/cuda/nvvm-nested-array-*.slang` fixtures: root arrays, guarded wrappers and guarded
  multidimensional arrays. Native directives select NVVM O0/O3; NVRTC's known failure remains explicit.
- Record contract, completed plan, structured validation and navigation document the qualified scope.
  Main frozen/discovery manifests are unchanged; new focused fixtures remain outside that corpus.

Focused validation passes: six native units, 24 native GPU cases, six flat-array neighboring cells and
two direct LLVM promotion cells. All three new fixtures pass NVVM O0/O3; NVRTC still returns wrong37.
Full validation preserves all 1,740 main cells and 576 input hashes: 1,703 correct and 37 unresolved,
with 20 resolved histories retained. Units pass 1,097 with 13 skips; semantic suites pass 1,170 with
78 skips, preserving every old identity. Runtime4/toolkit18/material6 and runner contracts pass.
All six material PTX/cubin pairs and parsed resources are identical to279. Candidate/final identities
match: compiler62469125, providerABI42 af1661de, source8fbf0f84e plus patch12f503e9.
The structured validation record retains exact outcomes and raw evidence under
`build/nvvm-nested-array-stores285`.

## Concepts and vocabulary

Store alignment is a guarantee about an address, not its physical layout. Alignment one is valid for
an address aligned to four or eight. A terminal store is one left intact after the existing provider
has split immediate struct fields. An SSA snapshot retains its value after its source memory changes.

## Process report

`NVVMTypeLoweringContext::_lowerArrayType` and `_lowerStructType` build canonical LLVM array/member
types through the provider. `emitNVVMIRFromLinkedIR` lowers the original store value and pointer;
`_emitStore` validates them before calling `_emitStorePreservingNestedStructLayout`. The three actual
before fixtures emit `[3 x Cell]`, `{i32, [3 x Cell], i32}` and `{i32, [2 x [3 x Cell]], i32}` stores at
alignment four. These are intentional supported representations, not malformed producer output.
Independent direct-libNVVM counterfactuals isolate the store annotation as sufficient for correction.

The new `_containsNestedStructLayout` follows canonical array element types once and struct members,
returning true for an immediate struct-in-struct boundary. It stops at pointer pointees. At its sole
terminal-store caller, a surviving nested boundary must be inside an array. Existing direct splitting
and `commonAlignment` remain intact. No existing admission predicate answers this physical-layout
question without following unrelated pointees. The new helper and alignment branch both survive the
input-shape audit: they translate valid input for the target compiler without inventing another type,
reloading a changed source, introducing helper arguments or unrolling array elements.

Before implementation, an annotation-only LLVM gate passes constructed values, snapshots taken before
source mutation, both phi choices and genuinely unaligned destinations with canaries. The actual Slang
fixtures independently compute every field expectation across all 65,536 low16-bit patterns. They
initialize destinations with different values before each copy; omission controls produce exactly
wrong1/wrong4. Before correction, NVVM O0 passes all three, NVVM O3 returns wrong37/wrong5/wrong5, and
NVRTC O3 returns wrong37 for all three. After correction, every NVVM output is `[0,123,0,456]`; NVRTC
failures are unchanged. Exact LLVM comparisons contain only two/one/one alignment4→1 edits.

The first nine fixture attempts failed parsing after formatting joined struct declarations without
semicolons; no GPU ran. Those outcomes and sources remain retained. Explicit terminators and protected
test directive lines fix the fixtures before the final frozen run. Review also fixed an output-header
parser before execution. The maintained classifier does not recognize this FileCheck CHECK-NEXT
failure form; raw infrastructure/unclassified labels remain alongside the independent full-buffer
runtime-mismatch assessment. The production harness is unchanged.

The provider changes neither arbitrary array admission nor the known large compilation limit: original
and annotation-only N65536 O3 modules both exceed 120 seconds/4 GiB. This is not proof of identical
causes, and large modules were never GPU executed. Conservative alignment changes some copy widths;
no compile-speed or GPU performance improvement is claimed. NVRTC repair remains separate. A fresh
author, separate reused-context reviewer and root audit the scope; the reviewer discloses prior279
implementation authorship. No Slack notification or publication is performed.

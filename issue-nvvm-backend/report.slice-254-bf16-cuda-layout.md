# Slice254: preserve BF16 identity in CUDA layout producers

Status: accepted full checkpoint, provider ABI41. Delegation was unavailable at the agent limit;
one parent owned the bounded implementation, with separate hand-derived oracle and mechanical
acceptance audits. No fresh independent-agent review is claimed. No push or system change occurred.

## Motivation

Consider this complete record and array shape:

```slang
struct Wrapped
{
    uint16_t prefix;
    vector<BFloat16, 4> value;
    uint16_t suffix;
};
struct Holder
{
    Wrapped values[3];
    uint16_t tail;
};
```

CUDA's prelude represents BF4 as four 2-byte component fields. `Wrapped` therefore occupies12 bytes,
with value/suffix offsets2/10; `Holder` occupies38 bytes with tail36. Before this change, public CUDA
reflection and explicit IR CUDA queries reported24-byte records, offsets8/16 and80-byte holders.
Research253 demonstrated the practical consequence: reading three records packed from reflection
returned17 wrong logical fields out of18 in both pointer and StructuredBuffer controls.

The unchanged253 query has five wrong direct results. The new96-value fixture also qualifies
row-major2x4/3x4 BF matrices, which inherit BF4's alignment. Before production edits, NVRTC matched
all96 expected values and both direct modes had15 wrong values. A separate CUDA-device probe
matched224 ABI words and32 untouched sentinel words. BF2/BF3, Half and ordinary integer/float vectors,
and row-major4x2/4x3 BF matrices provide neighboring controls.

## Proposed solution

Preserve the existing canonical element `Type*` through the internal AST vector/matrix layout
interface, then make AST and IR CUDA layout producers agree with the actual BF3/BF4 component
structs. Native BF2 remains4-byte aligned, Half3/Half4 retain their padding, and Natural rules remain
separate. Downstream reflection, explicit queries and aggregate offsets consume the corrected
producer result. No new scalar-kind mapping, alternate type, query-consumer workaround or runtime
storage admission is introduced.

## Change summary

- `slang-type-layout.h/.cpp`: pass `Type*` through vector/matrix rules and callers; remove the two
  lossy ordinary-layout BaseType extraction blocks. Keep BaseType for unchanged varying scalar
  rules. The synthetic uint2 descriptor query obtains the existing canonical UInt type from the
  context's ASTBuilder. All non-CUDA vector rules continue ignoring the semantic argument.
- `slang-ir-layout.cpp`: compute CUDA BF3/BF4 as tightly packed scalar components with scalar
  alignment. Existing vector/array construction propagates the corrected layout into aggregates
  and row-major matrices.
- `unit-test-special-scalar-reflection.cpp`: add public-reflection coverage for16 value types and
  their wrappers/array holders, including size, alignment, stride and field offsets.
- `nvvm-bf16-cuda-layout.slang`: add96 independently expected explicit CUDA size/alignment outputs
  in NVRTC O3 and NVVM O0/O3; append one discovery source. Frozen452 stays fixed.
- Plan, report, result manifests, design and STATUS retain the exact evidence and scope.

The optimized build passed. Runtime4 and focused5 pass. The new fixture returns all96 expected
values in each mode; reflection48 and IR CUDA36 agree with the actual ABI, while Natural36 remains
exact. The21 storage/query replay launches check5376 words, retain2 historical bad-input failures,
and verify6 corrected-reflection packing controls. All12 direct-storage rejection diagnostics and
all7 NVRTC PTX outputs remain exact253.

Full units pass1048 with13 skips, preserving all1060 previous identities and adding one test.
Semantic regressions pass1052 with77 skips, preserving all1129 identities. Toolkit18 and runner
contracts6 pass. Full frozen1356 preserves1347 correct and9 unresolved; discovery348 preserves
all345 old outcomes and adds3 correct cells. Combined1704/1665 correct/39 unresolved retains all18
resolved histories. All1701 previous cells match in classification, return code, execution counts,
diagnostic and canonical shape. All6 material compile/assembly support cells pass, with no material
runtime or speedup claim. Read [validation254](runtime-validation.slice-254.json) for exact identities,
comparison, commands and closure references. Raw evidence is under
`build/nvvm-loop/slice-254-before` and `build/nvvm-loop/slice-254-after`.

All6 material PTX and cubin outputs are byte-identical to252. Evidence closure verifies3509 indexed
raw artifacts,132 final source snapshots and201 final compact references. The final two references
name the closure script/result, which are added after the closure run's199-reference inventory.

The tested source is accepted research253 revision `11afb27a72a931ee0393205ca32df6384d276e7a` plus
this diff. Final identity covers132 source files,12 artifacts and564 runtime inputs; all563 old inputs
are unchanged. The compiler library is `a595092cb50be989df9015d38852946afd599def83b5485ba5188b2a5e4e3f7a`;
provider ABI41 remains `5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`. The broader
checkpoint is required by shared layout impact; no additional Debug build was needed for a new
assertion contract, because this change introduces none.

## Concepts and vocabulary

**Canonical element type** is the existing checked AST type. BF16 has `BFloat16Type`, whereas Half
is represented by `BasicExpressionType` with `BaseType::Half`; a BaseType alone cannot encode BF16.
**CUDA layout** describes storage in emitted CUDA C++ and explicit CUDA metadata queries.
**Natural layout** is a separate rule used by unqualified sizeof/alignof and is not a CUDA ABI oracle.
**Wrapped record** places uint16 prefix/suffix fields around a value so incorrect alignment becomes
observable in field offsets, record stride and arrays. **Storage admission** determines which IR
runtime types the direct backend accepts; correcting metadata does not admit those types.

## Process report

The checked source above reaches `_createTypeLayout` with a canonical vector element
`BFloat16Type`. Previously, extracting a BaseType succeeded only for `BasicExpressionType`, leaving
Void for BF16. `CUDALayoutRulesImpl::GetVectorLayout` therefore applied the ordinary four-lane
alignment rule to BF4. Keeping the original Type* lets that producer select the actual component
layout. Reflection's wrapped-record fields and array strides now derive from the same semantic
source of truth. Reconstructing BF16 from its two-byte size would confuse it with Half and UInt16;
adding another BF16 BaseType or patching each reflection consumer would duplicate the type model.

The IR producer already receives canonical `kIROp_BFloat16Type` but also lacked the prelude-specific
vector rule. `CUDALayoutRules::calcSizeAndAlignment` now computes the two qualified component widths
from scalar layout. The existing size/alignment and aggregate-offset consumers then use that result.
Explicit `__sizeOf`/`__alignOf` target queries reach `_getNVVMCUDALayoutQueryValue` and the CUDA IR
rule before runtime-value admission. The executable fixture thus proves the IR producer independently
of whether runtime BF vector storage is supported. The separate public-reflection test proves the
AST producer. Their input shapes are intentional canonical types, not malformed producer output;
the loss of semantic identity at the AST query boundary was the representation defect.

Helper/fallback inventory: no new production helper or fallback. The new AST BF3/BF4 branch and
matching IR branch survive because actual CUDA device layout independently qualifies those types.
The Half predicate changes only to read its original BaseType from canonical Type*. Removing either
CUDA correction restores its corresponding reflection/query mismatch. BF2 and unsupported widths
retain the previous behavior. The two obsolete BaseType extraction blocks are removed, while the
varying scalar queries retain the BaseType they still require. The descriptor query uses the
canonical UInt type from its existing populated layout context; no new representation is created.

The initial fixture used a short `type: uint` FileCheck header, which allowed its first expected2
match to land inside `uint32_t`. That failed preparation is retained. The corrected header and
fully anchored numeric expectations were frozen before production validation. A second direct
whole-output audit compares all96 integers. The guarded edit script's implementation-count assertion
also stopped once; the retained correction applied the implementation edits after the already
changed header. No failed preparation is counted as a passing compiler test.

The existing253 wrong-input buffers are preserved byte for byte as negative controls. New host
packing derived from final reflection must equal253's immutable correct CUDA-packed bytes; this
avoids silently rewriting old failing inputs. The12 direct runtime storage rejections must remain
unchanged. Material shader compile/assembly reassessment is required, but its missing binding,
texture/LUT, input and output contract still precludes material runtime or performance claims.

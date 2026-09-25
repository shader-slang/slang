# Qualify the CUDA BF16 layout mismatch

## Motivation

The BF4 mismatch recorded by research240 has an observable consequence on the accepted252 compiler.
Consider a host packing records using Slang's public CUDA reflection:

```slang
struct W
{
    uint16_t prefix;
    vector<BFloat16, 4> value;
    uint16_t suffix;
};

[CUDAKernel]
void computeMain(
    uniform StructuredBuffer<W> records,
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output)
{
    if (cudaThreadIdx().x != 0) return;
    for (uint i = 0; i < 3; ++i)
    {
        output[6*i] = uint(records[i].prefix);
        for (uint j = 0; j < 4; ++j)
            output[6*i+1+j] = uint(bit_cast<uint16_t>(records[i].value[j]));
        output[6*i+5] = uint(records[i].suffix);
    }
}
```

Reflection reports a24-byte record with value/suffix offsets8/16. The emitted CUDA struct actually
has size12 and offsets2/10. Three records packed from reflection produce17 wrong field values out
of18; the same logical values packed according to the actual CUDA ABI are exact. This is an
isolated control, not a changed corpus input or material runtime claim.

## Proposed solution

Keep253 as research and repair the two CUDA layout producers in the next bounded implementation.
Preserve the canonical BF16 type and carry its format into the AST vector-layout query rather than
losing it as BaseType::Void. Make the IR CUDA rule describe the prelude's BF3/BF4 component structs.
Both producers must preserve BF2's native4-byte alignment, Half's separate padding, ordinary
integer vectors and all natural-layout behavior. Do not change the emitter, relax aggregate checks,
coerce BF16 into Half/UInt16, or admit additional physical storage roles to conceal the mismatch.

The repair has an immediately runnable direct-backend regression without requiring BF16 storage:
`__sizeOf<W>()` and `__alignOf<W>()` are existing compile-time CUDA queries whose results are ordinary
integers. Both direct modes currently return five wrong BF4 values in a72-value query control.
NVRTC matches independently measured CUDA layout. The saved accepted250/pre252 compiler emits
byte-identical PTX in all three modes, proving this predates252's constructor optimization.

## Change summary

Only this report, the completed plan, compact semantic evidence, the BF16 design contract and STATUS
change. Production source, provider ABI41, binaries, corpus manifests, fixtures and all563 existing
runtime inputs remain exact252. Raw controls live in `build/nvvm-loop/slice-253-bf16-storage-layout`.

| Evidence                               | Result                                                                                        |
| -------------------------------------- | --------------------------------------------------------------------------------------------- |
| Small runtime gate                     | 4 correct                                                                                     |
| CUDA device layout / public reflection | 36 rows each; only BF4's vector, wrapper and array-holder rows disagree                       |
| Canonical IR layout                    | 72 CUDA/Natural rows; CUDA agrees with reflection on all36 rows                               |
| Pointer / StructuredBuffer controls    | 12 launches;10 exact logical outputs,2 measured reflection-packing failures                   |
| Explicit CUDA layout queries           | 3 launches; NVRTC correct, NVVM O0/O3 each have5 wrong values                                 |
| CUDA ABI probe                         | 1 launch,168 metadata words and88 sentinels exact                                             |
| Complete final output audit            | 16 launches,4096 returned words; exact byte predictions, including the recorded failures      |
| Pre252 comparison                      | 3 fresh compiles, all PTX byte-identical to current queries; no historical GPU replay claimed |

The table excludes two completed preparation launches retained with the initial harness assertion
failure. There are also12 explicit direct preflight stops from the pointer/buffer controls. None
counts as a runtime pass. The registered full252 corpus is inherited:1701 cells,1662 correct,
39 unresolved and18 resolved histories. No new corpus identities, full-checkpoint reset or cadence
advance. The original dynamic-dispatch fixture uses BF2 and retains independent aggregate/storage
obstacles; this research does not claim to resolve it.

## Concepts and vocabulary

- **CUDA layout:** actual target ABI size, alignment, field offsets and array stride, as consumed by
  CUDA reflection and direct backend CUDA query folding.
- **Natural layout:** the separate rule selected by unqualified Slang `sizeof`/`alignof`. It is not
  evidence for CUDA ABI equivalence; BF2 demonstrates the distinction.
- **Compile-time query:** the core module's `__sizeOf`/`__alignOf` helper carries a type as metadata;
  its result can be supported without admitting that type as a runtime backend value.

## Process report

The production helper/fallback/special-case inventory is empty. The research sources query public
reflection, construct canonical IR types, inspect actual prelude ABI and execute exact byte probes.
All are retained under ignored build paths. Fresh-agent delegation failed with `agent thread limit
reached`; WORKFLOW's explicit local fallback was used. The parent was the only writer and performed
a separate hand-derived oracle audit. No fresh independent agent review is claimed.

`_createTypeLayout` receives the intentional `VectorExpressionType(BFloat16Type,4)`. Its scalar
branch requests UInt16's size/alignment while preserving the actual type in TypeLayout. The vector
branch extracts BaseType only for BasicExpressionType; BF16 therefore reaches
`CUDALayoutRulesImpl::GetVectorLayout` as BaseType::Void. The generic CUDA vector rule gives
size8/alignment8. This is a loss of format information at the layout-query boundary, not a bad AST
type that needs reconstruction. Reflection then reports `W` as24/8 and its three-record holder as80/8.
The actual prelude defines BF4 as four scalar BF16 members:8/2, W12/2, holder38/2, tail offset36.

The standalone linked-object probe constructs the corresponding IR scalar/vector/struct/array
types with IRBuilder and calls `getSizeAndAlignment` and `getOffset` for explicit CUDA and Natural
rules. Null target is supported by `getBuiltinTypeLayoutInfo` for these pointer-free scalar sizes;
the selected rule supplies vector/aggregate behavior. Matching optimized objects and link hashes
are retained. IR `CUDALayoutRules::calcSizeAndAlignment` handles Half specially but delegates BF4 to
the generic vector rule, reproducing reflection's10 differing scalar fields across3 layout rows.
All33 neighboring CUDA rows agree with the device probe.

For `__sizeOf`/`__alignOf`, `core.meta.slang` produces intentional target-selected GenericAsm
helpers. `_getNVVMCUDALayoutQuery` recognizes their canonical shapes, and
`_getNVVMCUDALayoutQueryValue` calls the same CUDA layout producer before runtime type admission.
The direct query control returns BF4 alignment8, W size24/alignment8 and holder size80/alignment8
instead of2,12/2 and38/2. The five mismatching indices are19 through23. Fixing these constants in
the query consumer would leave reflection and other layout consumers wrong; the producers own it.

Unqualified `sizeof`/`alignof` follows `PeepholeContext::processInst`'s Natural rule. The initial probe
incorrectly expected CUDA values for BF2 and failed only at its three metadata outputs; all field
reads were exact. The retained corrected protocol checks Natural values separately without changing
shader source, input packing or logical field expectations. This is why the next repair must not
blindly change Natural rules. An initial NVCC host-executable probe also failed in BF16 math overload
checks; the final device-only SM80 probe compiles and executes the actual prelude. Host-executable
NVCC support was not qualified or repaired.

The runtime audit covers signed zero, subnormal, finite, infinity and NaN bit patterns with exact
uint16 transport, three records, adjacent prefix/suffix values and untouched input/sentinel bytes.
BF2/BF3 reflection and actual packing are byte-identical controls. BF4's two packings preserve the
same logical oracle; its wrong reflection case remains a failure. The StructuredBuffer version uses
the actual16-byte pointer/count parameter and reproduces the pointer control's complete output.
Direct pointer controls reject the canonical pointer reinterpretation; uniform StructuredBuffer
controls reject the entry parameter. These are separate recorded support limits, not proof of new
storage support. No additional independent blocker was investigated.

Native Ubuntu24.04, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14 and ABI41
remain fixed. Compiler `10ffeb3246d56c9a835b1cd606e35a1a2cd6c8f9fcb3f6bfef36fed26413ea7b` and provider
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab` remain unchanged. Sequential
bounded GPU controls ran without device loss, system changes, reboot or push. Full252's runtime,
unit/semantic/toolkit and material evidence remains historical, not fresh253.

The next repair should add the query control as runnable regression coverage and test public
reflection's BF2/BF3/BF4 wrapper/array layouts, while preserving all neighboring and Natural results.
Its shared layout impact requires a full checkpoint. Any later storage admission needs its own
role/physical-layout proof; it is outside that producer repair.

Local acceptance verifies the unchanged identities, all layout/query/control results,131 repository
source snapshots covering130 unique paths,318 indexed artifacts and83 compact references. A separate
closure audit also verifies all36 Natural rows from an independent size/stride formula. Its two
acceptance artifacts are referenced outside the closed research index. The closed raw index retains
all failed preparation evidence. Research253 is accepted; implementation/full252, targeted233 and
cadence0 remain authoritative.

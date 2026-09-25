# Establish narrow-integer min/max prefix semantics

## Motivation

After slice 227, the original frozen prefix minimum and maximum workloads stop at canonical
`int8_t(int8_t, vector<uint,4>)` exclusive prefixes. Consider this reduced dynamic workload:

```slang
[CUDAKernel]
void computeMain(uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> data)
{
    uint lane = cudaThreadIdx().x;
    uint mask = uint(data[0]);
    if ((mask & (1u << lane)) != 0)
    {
        int8_t value = int8_t(data[32 + lane]);
        int8_t result = WaveMultiPrefixExclusiveMin(value, uint4(mask, 0, 0, 0));
        data[64 + lane] = int(result);
    }
}
```

For mask 3, lane 0 input -128 and lane 1 input 127, the expected results are 127 and -128.
The first participant receives the exact signed 8-bit maximum, and the second receives its
predecessor's value. A uint8 version instead has identity 255 and interprets input bit 7 as positive.
A correct extension must preserve both that identity and the signedness of comparison.

## Proposed solution

This slice establishes the contract without changing compiler code. Independent integer expectations
check source execution, while minimal direct probes identify the responsible rejection layer.
The next bounded implementation should admit narrow integer prefix min/max with width-derived
identities and reuse existing typed lane transport, integer min/max and aggregate leaf recipes.
The identity helper is shared with reductions and arithmetic: keep the newly admitted domain
explicitly bounded rather than widening its width guard for all operations. No provider or ABI
change is needed by the demonstrated signatures.

## Change summary

- `semantic-evidence.slice-228.json` records identity verification, fresh research counts, source and
  direct compile evidence, output hashes, the independent oracle, and inherited registered results.
- The completed plan, this report and STATUS record the semantic findings and next bounded action.
- Generated scalar/vector probes, CUDA source, direct IR, PTX/cubins, scripts and full per-launch
  hash records stay in `build/nvvm-loop/slice-228-narrow-prefix`.
- No compiler, provider, tests, corpus manifests or application shader changed. No commit or push
  was made by the worker.

## Concepts and vocabulary

A _prefix member_ is a selected lane before the caller, including the caller for an inclusive
operation. The _exclusive identity_ is the value returned when that set is empty. _Typed lane
transport_ preserves the original low bits through a 32-bit shuffle and reconstructs the original
semantic type. Its word-extension rule does not decide the later comparison's signedness.

## Process report

`hlsl.meta.slang` generates the scalar and vector `WaveMultiPrefixInclusive/ExclusiveMin/Max`
overloads from `kWaveMultiPrefixMinMaxNames`. The scalar CUDA branch produces
`_wavePrefixExclusiveMin(($1).x, $0)`; vectors produce the corresponding `Multiple` spelling.
The retained final checked IR contains a `Func(Int8, Int8, Vec(UInt,4))` helper with that GenericAsm.
This is intentionally valid canonical data. There is no accidental alternative AST/IR spelling to
repair and no semantic value that needs reconstruction as syntax.

For the example above, generated CUDA uses signed `char` and calls `_wavePrefixExclusiveMin`.
The four narrow source representations are `char`, `uchar`, `short` and `ushort`, including vectors.
`WaveOpExclusiveMin/Max` delegates to `WaveOpMin/Max`; `_wavePrefixScalar` and `_wavePrefixMultiple`
implement shuffle-up stages for low contiguous power-of-two masks and ascending original-input
scans for other masks. Inclusive initialization retains the caller. Exclusive initialization uses:

| Type     | Minimum identity | Maximum identity |
| -------- | ---------------: | ---------------: |
| int8_t   |              127 |             -128 |
| uint8_t  |              255 |                0 |
| int16_t  |            32767 |           -32768 |
| uint16_t |            65535 |                0 |

All four narrow integer types promote to `int` for the `<` or `>` comparison. The conditional
selection chooses one original operand; it does not add, multiply, or overflow. The selected value
is already representable in the original type. CUDA shuffle overloads transport promoted values,
then assignment converts the result back to the narrow type. Seven CUDA compile-time assertions
confirm signed `char`, the five relevant narrow promotions including `int8_t`, and the contrasting
unsigned 32-bit promotion rule. A separate host check exhausts all 65,536 signed 8-bit operand pairs
and all 65,536 unsigned pairs for both comparisons against independently decoded integer values.
These host pairs are supplemental evidence, not GPU pair coverage.

Consequently integer min/max is associative and idempotent, and equal values have equal bits.
Unlike the prior FP64 NaN/signed-zero case, tree-versus-scan operand order cannot change the result.
The oracle therefore computes the mathematical minimum/maximum of the mask-selected prefix set,
using explicit type extrema for an empty set. It does not simulate CUDA's algorithm, call another
wave intrinsic, or take NVRTC output as the expected answer. Signed decoding uses the low N bits
and subtracts 2^N when the sign bit is set. Output comparisons include sign/zero extension into
32-bit words, and extra wide-input cases check the initial truncating conversion.

The direct consumer traces through `_resolveNVVMMaskedWaveScalarOperation`, or
`_resolveNVVMAggregateWaveOperation` and its homogeneous leaf resolver, into
`_initializeNVVMMaskedWaveScalarOperation`. `_getNVVMMaskedWaveScalarIdentity` rejects widths other
than 32 and the selected FP64 case before the recipe's remaining operations are admitted. All
64 minimal direct probes reproduce E52017: four types, four operations, scalar/vector4 and O0/O3.
All produce no PTX. The original signed 8-bit exclusive minimum and maximum signatures are retained
explicitly; no next independent frozen blocker was investigated.

The semantic catalog already admits narrow `MIN`, `MAX`, `WAVE_READ_LANE_AT` and `SELECT` for all
four types; 16 standalone contract checks pass. Provider `_emitWaveReadLaneAt` zero-extends the
original bits to i32, invokes `nvvm_shfl_sync_idx_i32`, and truncates the result to the original
width. Zero extension is correct even for signed values because this operation transports bits.
The `IntegerBinary` min/max family then selects `ICMP_SLT/ULT/SGT/UGT` from semantic signedness
and applies select at the original width. No promoted comparison or transport patch is required.
The existing `_emitNVVMMaskedWaveScalarValue` scan and inclusive `>=` / exclusive `>` membership
predicates can implement the integer contract once exact identity admission is supplied.

There are no new production helpers, fallbacks or special cases to audit or revert. The research
helpers are a mathematical oracle, probe generator, CUDA driver scaffold, and evidence summarizer.
They do not perform compiler substitution, resolution, syntax reconstruction or graph matching.
The failing minimal tests establish the admission boundary; source/provider inspection and the
catalog checks explain why that boundary owns the missing functionality. A future implementation
must preserve narrow source/expected inputs and prove direct execution before claiming support.

Validation uses the unchanged accepted compiler and provider, after checking all 27 source hashes,
12 artifact/toolkit hashes and 552 registered input hashes. The fresh smoke gate passes 4/4.
Fresh research results are separate from the registered corpus:

| Research coverage                                        | GPU launches | Result    |
| -------------------------------------------------------- | -----------: | --------- |
| Signed/unsigned 8-bit, NVRTC O3                          |       14,672 | All exact |
| Signed/unsigned 16-bit, NVRTC O3                         |          784 | All exact |
| Signed/unsigned 32-bit controls, NVRTC O3 and NVVM O0/O3 |        2,016 | All exact |
| Total                                                    |       17,472 | All exact |

The total comprises 17,248 primary launches plus 224 wide-input truncation launches. Every launch
checks four operations at scalar/vector2/vector4 widths: 28 results per lane, 896 output words.
There are exactly 5,136,768 active integer comparisons and 10,518,144 inactive sentinel comparisons,
15,654,912 words overall, with input buffers unchanged. Every 8-bit value is exercised at every
active caller under every mask by 256 constant patterns, supplemented by 256 permutations and
boundary patterns. This does not exhaust all GPU input tuples. Fourteen masks cover full, low
power-of-two, irregular, high, alternating, sparse extremes and singleton lanes 0/7/31.
All ten runtime PTX artifacts assemble. The extra CUDA promotion assertion cubin is a compile check,
not another runtime launch or PTX assembly in those counts. No cases are missing or duplicated.

Registered evidence is wholly inherited: 1668 cells, 1617 correct, 51 known failures and six
resolved histories. Slice 227 measured 633 cells freshly and inherited 1035 from full checkpoint 225. This research adds zero registered cells, leaves discovery at 104 identities and leaves
implementation cadence at one. No full corpus replay is warranted for this unchanged compiler.
No material runtime or performance claim is made; application bindings, textures/LUTs, inputs and
expected output remain unavailable. GPU health remains intact; no driver/system change or reboot.

The compiler library remains SHA-256
`9e4b11ac87a9cd4941a4bee64855ae0a8008208680801c0f0c8cabbf51946846` and provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36.
The tested source is `b7f2541f7a71312a2e7f3d98047e8a09a0453c22` on `nvvm-backend`.
Parent independently accepted the research after checkout ownership returned; completed records
are included in the authorized local commit.

2026-09-25 parent acceptance: independently reviewed the mathematical oracle, complete launch
inventory/output hashes and exact rejection logs. Verified 187 evidence references, 27 source hashes,
12 artifact hashes and 552 unchanged registered inputs. Registered ledger/cadence remain unchanged.

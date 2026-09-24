# Slice 215: stop FP64 min/max research on a supported FP32 singleton defect

## Motivation

The two frozen `hlsl-intrinsic/wave-multi/wave-multi-min-max.slang#cuda-1` NVVM cells stop at
`_waveMin($1.x, $0)`, signature `double(double,uint4)`. Their finite 15/17-lane scalar/vector
inputs and all-one output oracle remain unchanged. FP64 min/max admission was deliberately
excluded from slice 208 because CUDA's comparison/select reduction and numeric min/max differ.
This research gate first checked the already supported FP32 path and found a smaller correctness
issue. No production change or new registered corpus coverage is included in this slice.

Consider this complete dynamic-input reproduction:

```slang
[CUDAKernel]
void computeMain(uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> data)
{
    uint lane = cudaThreadIdx().x;
    uint mask = uint(data[0]);
    float value = asfloat(uint(data[32 + lane]));
    if ((mask & (1u << lane)) != 0)
    {
        data[64 + lane] = int(asuint(WaveMultiMin(value, uint4(mask, 0, 0, 0))));
        data[96 + lane] = int(asuint(WaveMultiMax(value, uint4(mask, 0, 0, 0))));
    }
}
```

Launch one 32-thread block with 128 device words initialized to `0xdeadbeef`, except
`data[0] = 0x80000000` and `data[63] = 0x7fc12345`. Lane 31 is the sole named participant and
executes both same-mask operations; other lanes bypass them and are not named. Read words 95/127.
The independent source-helper oracle returns the input word unchanged for each operation. NVRTC
O3 does so; NVVM O0/O3 return `0x7f800000` / `0xff800000`, respectively. A signaling input
`0x7f812345` produces the same defect. Neither input nor mask is a compiler constant.

## Proposed solution

Stop the FP64 admission gate at the explicit supported-defect stopping condition. Prioritize
slice 216: preserve the original FP32 operand for singleton min/max **reductions**, using the
existing typed singleton-preservation machinery already used by FP64 arithmetic where appropriate.
The smallest demonstrated domain is singleton scalar FP32 min/max. The same scalar recipe serves
aggregate leaves, so the next slice must cover scalar, vector and lowered matrix forms with raw-bit
oracles, plus unchanged non-singleton neighbors. Keep FP64 min/max admission and other reduction
operations outside that fix. A singleton selection is principled because the source performs no
combine at all; changing the provider's numeric min/max globally would alter an unrelated contract.

This does not decide the larger FP64 NaN/order contract. Existing typed ordered float comparisons,
selects, lane indices, bit operations and exact 64-bit indexed shuffles appear sufficient to express
both source branches without a new provider ABI. An XOR source lane can use the existing indexed
shuffle with `lane ^ offset`. That is a static feasibility observation, not a tested FP64 recipe.
The broader independent algorithm model and GPU matrix must resume after the correctness slice.

## Change summary

- Completed research plan: bounded matrix, original acceptance criteria and conditional stop.
- This five-part report and `semantic-evidence.slice-215.json`: first-known reproduction,
  measured results, exact provenance, inherited ledger and remaining research.
- `STATUS.md`: prioritize the FP32 singleton fix before resuming FP64 min/max admission.
- Ignored `build/nvvm-loop/slice-215-semantics/`: dynamic shader, ctypes driver apparatus,
  per-mode PTX/cubins/logs, canonical IR, raw-word comparisons and identity verification.

No compiler, provider, prelude, test source, runner, shader contract, corpus manifest or ABI changed.
No compiler build, commit, push or system change occurred.

## Concepts and vocabulary

A singleton partition has exactly one mask bit set and contains its caller. The CUDA helper's
singleton path is an identity: it returns that caller's original bits. A numeric min/max operation
chooses a numeric operand over a NaN operand. A reduction seed is an extra initial accumulator;
its neutrality depends on the operation and input domain. A canonical masked-wave recipe is the
backend's typed implementation of the checked specialized CUDA helper, shared by aggregate leaves.

## Process report

The helper/fallback inventory contains no new production entries. The probe adds only one ignored
kernel and an ignored driver/oracle script. Its expected words come from the source singleton
identity, not from NVRTC output. No host floating-point min/max or NaN conversion is used.

`hlsl.meta.slang` lines 18935 onward produce scalar `WaveMultiMin/Max` GenericAsm helpers. The
captured final IR contains `Func(Float, Float, Vec(UInt,4))` and `_waveMin($1.x,$0)` /
`_waveMax($1.x,$0)`. Vector overloads use `Multiple`; matrices use the same helper after aggregate
lowering. These are valid canonical source representations; no producer reconstruction is needed.

In `prelude/slang-cuda-prelude.h`, `_waveCalcPow2Offset` accepts the full mask or a contiguous run
starting at bit zero whose population is a power of two. High aligned subgroups do **not** qualify.
`_waveReduceScalar` and `_waveReduceMultiple` then implement the following source algorithm:

1. A qualifying mask starts with each caller's value and combines simultaneous XOR neighbors at
   decreasing offsets `size/2, size/4, ..., 1`.
2. A nonqualifying nonsingleton starts from each caller's value. It combines original input values
   from named lanes in ascending lane order; aggregate components use the same sequence separately.
3. A singleton returns the original operand unchanged. Low-lane singleton mask 1 also executes
   zero XOR stages. High singleton mask `0x80000000` bypasses the irregular loop.

`WaveOpMin/Max::doOp` compares with `<` / `>` and returns the second operand when the comparison
is false, including equality and unordered NaN inputs. Thus caller, order and selected zero/NaN
words can matter for nonsingletons. Those broader cases were deliberately not dispatched after
this slice's stopping condition fired.

Direct NVVM accepts the FP32 shape in `_resolveNVVMMaskedWaveScalarOperation` and
`_initializeNVVMMaskedWaveScalarOperation`. `_getNVVMMaskedWaveScalarIdentity` supplies positive
infinity for min and negative infinity for max. `_emitNVVMMaskedWaveScalarValue` scans the mask
and combines the accumulated value with each shuffled original operand. Its singleton preservation
is guarded by `preservesFloat64Reduction`; the FP32 min/max recipe does not activate it. The
provider FloatBinary MIN/MAX maps through `_emitLibdeviceOperation` to `__nv_fminf/__nv_fmaxf`.
The [NVIDIA libdevice contract](https://docs.nvidia.com/cuda/libdevice-users-guide/__nv_fminf.html)
chooses the numeric argument when only one argument is NaN. Consequently the injected infinity
survives the one-lane reduction. The provider obeys its own numeric contract; the masked recipe
has selected the wrong source algorithm for this valid shape.

Actual O0/O3 PTX contains the respective infinity seed, indexed shuffle, and `min.f32` / `max.f32`.
NVRTC PTX contains ordered `setp.lt.f32` / `setp.gt.f32` and `selp.f32`, butterfly and indexed
branches, and the singleton bypass that preserves the loaded word. All three PTX artifacts
assemble for SM80. The NVVM O0 IR-dump compile also reproduces byte-identical PTX to the measured
O0 artifact. Dynamic `ld.global` inputs, branch membership and `st.global` outputs remain visible.

Contract distinction: local `docs/wave-intrinsics.md` relates WaveMulti to partitioned subgroup
operations but does not promise cross-target NaN payloads, signed-zero tie order or reduction
order. Microsoft's [WaveActiveMin documentation](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/waveallmin)
explicitly leaves operation order undefined; it is a different API and is not imported as a
stronger WaveMulti rule. This finding is a CUDA source-helper compatibility defect on the direct
backend, not a claim that all portable backends must reproduce CUDA's exceptional-value ordering.
For the measured singleton there is no reduction ordering to choose: the existing CUDA helper
returns its input without arithmetic. NaN becoming infinity is not a payload-only difference.

| Fresh gate                         | Result                                                                    |
| ---------------------------------- | ------------------------------------------------------------------------- |
| Existing small runtime smoke       | 4/4 correct                                                               |
| Singleton independent-oracle cases | 12 executions: 8 correct, 4 mismatches                                    |
| NVRTC O3                           | finite 1, negative zero, quiet NaN, signaling NaN all preserve both words |
| NVVM O0 and O3                     | finite 1 and negative zero correct; each NaN gives +inf min / -inf max    |
| Raw min/max output comparisons     | 24 words: 16 equal, 8 unequal                                             |
| PTX assembly                       | 3/3 SM80                                                                  |
| Preserved accepted identities      | 15 source hashes, 12 artifact hashes, 546 runtime-source hashes           |

First-known evidence is this slice on base `357d270587d132e1de92060b51e83f6a9be062b2`, native
Ubuntu 24.04, L4 SM89 / driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM14,
provider ABI36. Compiler SHA256 is
`2775a5783a7dd1310ab9773d259bf1bdafc464a4f55a376c1fc22c472a6b1bf0`; provider SHA256 is
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
These match accepted 214 exactly. The defect exists on that accepted artifact; its introducing
revision has not been bisected and no regression claim against earlier binaries is made.

Reproduce after sourcing `build/nvvm-loop/slice-203-env.sh` with
`timeout --kill-after=10s 180s python3 build/nvvm-loop/slice-215-semantics/probe.py`.
The driver validates accepted 214 hashes, compiles the dynamic shader in the three modes, allocates
and copies raw words, launches 32 threads, synchronizes, reads exact output words and destroys its
context. Each mode retains its full compile command and actual outputs in `singleton-results.json`.

The accepted 214 ledger is inherited unchanged: 1,650 registered cells, 1,597 correct, 53 open
failure records and four resolved histories. This newly observed research defect is separate and
must not be silently added to or substituted for those 53 records. Latest implementation/full
checkpoint is 214; cadence remains zero. Units 477 plus one existing Windows-only skip and toolkit
18 inherit 214/213; six material cells inherit compile/assembly support only. No material runtime
bindings or oracle exist. The two original FP64 preflights also inherit 214; they were not rerun
because the explicit supported-defect stop superseded further investigation. No FP64, aggregate,
full-warp, irregular, mixed-NaN or tie-order GPU result is claimed for this research slice.

Parent acceptance, 2026-09-24: independently reviewed the dynamic-input reproduction, all twelve
result rows and generated NVVM PTX; verified all 21 baseline/raw evidence hashes and exact research
counts. Accepted as research at the predeclared supported-defect stop. Latest full checkpoint 214
and implementation cadence zero remain unchanged. Slice 216 owns the bounded singleton fix.

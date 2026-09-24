# Slice 217: audit nonsingleton masked floating min/max

## Motivation

Consider this scalar portion of the measured kernel. Every named lane executes both calls, and the
host supplies only quiet NaNs, with distinct payloads and alternating signs:

```slang
uint lane = cudaThreadIdx().x;
uint mask = uint(data[0]); // 0xffffffff supplied by the host
if ((mask & (1u << lane)) != 0)
{
    float value = asfloat(uint(data[32 + 4 * lane]));
    data[160 + lane] = int(asuint(WaveMultiMin(value, uint4(mask, 0, 0, 0))));
    data[192 + lane] = int(asuint(WaveMultiMax(value, uint4(mask, 0, 0, 0))));
}
```

The CUDA source helper always selects an input word. With exclusively NaN inputs, no reduction
order can produce a non-NaN through that comparison/select operation. Accepted 216 NVVM O0/O3
instead returns positive infinity for minimum and negative infinity for maximum. Lane 0's expected
word is `0xffc00020` for both operations; its actual words are `0x7f800000` and `0xff800000`.
Singleton preservation from 216 remains correct. This gate checks the remaining algorithm before
FP64 admission and makes no new cross-target payload guarantee.

## Proposed solution

This slice records research only. Select one subsequent FP32 masked min/max recipe correction:
use caller values, ordered comparisons and typed selection, and reproduce the source helper's
butterfly versus ascending-scan distinction. Reuse existing mask, lane, indexed-shuffle, comparison,
select and control-flow operations; no provider ABI change is expected. Keep FP64 admission,
prefixes, sum/product and ordinary numeric min/max outside that implementation slice.

Changing only the final result when all inputs are NaN would hide the algorithm mismatch. Starting
from a caller value but retaining numeric min/max would still change mixed-NaN and zero behavior.
Replacing numeric min/max with comparisons while retaining an unconditional ascending scan would
still change the low contiguous power-of-two butterfly's selections. The principled layer is the
recipe translating these particular CUDA helpers. No front-end representation defect was found.

## Change summary

Only the completed plan, this report, semantic evidence and STATUS are durable changes. The dynamic
shader, integer oracle, CUDA driver apparatus, 288 matrix executions, retained-baseline replay and
three new PTX/cubin pairs live under `build/nvvm-loop/slice-217-semantics/`. No compiler, provider,
prelude, runner, registered corpus or source oracle contract changed.

## Concepts and vocabulary

A _source-helper compatibility result_ compares the direct backend against the concrete CUDA helper
selected by its canonical GenericAsm spelling. It is more specific than a portable wave intrinsic
promise. A _butterfly stage_ reads all lanes' prior states simultaneously through XOR partners.
An _ascending scan_ reads original values from increasing named lanes. _Numeric min/max_ prefers a
non-NaN when paired with a NaN; the source helper's ordered comparison instead selects its second
operand for an unordered comparison. An _injected seed_ is a value absent from the input set.

## Process report

The fresh-context worker spawn failed with `agent thread limit reached`. Under WORKFLOW's explicit
local fallback, the parent executed this bounded research and reviewed its apparatus. This limits
review independence and is recorded rather than implying a fresh worker performed it.

The canonical producer in `hlsl.meta.slang` selects `_waveMin($1.x, $0)` / `_waveMax($1.x, $0)` and
corresponding Multiple helpers. `WaveOpMin/Max` in `prelude/slang-cuda-prelude.h` uses `a < b ? a : b`
or `a > b ? a : b`; reduction `getInitial` returns the caller value. `_waveCalcPow2Offset` admits
only a low-bit contiguous power-of-two population. `_waveReduceScalar/Multiple` then runs XOR
stages from population/2 down to 1, using previous-stage values. Other nonsingletons start with the
caller and scan original named-lane inputs in ascending order. Singletons return the original word.
The aggregate implementation copies original components before scanning, preserving the same rule.

`_resolveNVVMMaskedWaveScalarOperation` / `_resolveNVVMAggregateWaveOperation` accept these valid
helper shapes and reuse `NVVMMaskedWaveScalarOperation` for scalar leaves. In direct emission,
`_getNVVMMaskedWaveScalarIdentity` supplies infinity, and `_emitNVVMMaskedWaveScalarValue` performs
an ascending scan through the numeric MIN/MAX operation. The provider correctly maps that operation
to libdevice numeric min/max. Generated O3 PTX visibly initializes `0f7F800000` / `0fFF800000`, then
executes `shfl.sync.idx.b32` and `min.f32` / `max.f32`. The 216 final singleton select is present.
The consumer recipe is wrong for the concrete source algorithm; neither a new semantic representation
nor a global provider behavior change is warranted.

The independent oracle uses raw integer IEEE words: NaNs are identified by exponent/fraction;
both zeros compare equal; other words use sign-aware integer ordering. It returns the selected
original word without host floating arithmetic. Hand checks cover infinities, signed finite values,
zeros, unordered operand positions, singleton signaling payloads, a two-lane simultaneous butterfly,
and a sparse ascending scan. The generated GPU shader loads four dynamic words per lane and checks
scalar, float2 and float2x2 operations. Each launch has one 32-thread block. The apparatus also checks
unchanged input memory and sentinel outputs for every unnamed lane.

The fixed matrix has eight masks (full, low16, high16, low15, high17, even, odd, singleton31) and twelve
families: finite, infinities, alternating signed zeros, all quiet NaNs, all signaling NaNs, mixed NaNs,
and one quiet/signaling NaN at the first/middle/last named lane among finite values. All 96 cases run
in each of NVRTC O3, NVVM O0 and NVVM O3. Every distinct emitted PTX assembles for SM80.

| Mode     | Exact cases | Active words equal | Active words compared |
| -------- | ----------: | -----------------: | --------------------: |
| NVRTC O3 |       96/96 |             21,672 |                21,672 |
| NVVM O0  |       48/96 |             13,216 |                21,672 |
| NVVM O3  |       48/96 |             13,216 |                21,672 |

All finite, infinity and singleton controls pass. Each NVVM mode has 8,456 differing active words:
7,784 expected NaN words become numeric, and 672 signed-zero words differ. All-NaN families fail
on all seven nonsingleton masks, affecting scalar and aggregate components. For a mode, the three
all-NaN families account for 5,376 differing words, all becoming infinity. Positional-NaN and
signed-zero differences are exact source compatibility observations, not an assertion that every
wave backend must choose identical order or payloads. All 64,008 inactive output sentinel checks
across the 288 launches pass. Total active words are 65,016, with 48,104 equal and 16,912 different.

NVIDIA documents numeric min/max as selecting the numeric operand over a NaN and returning NaN
when both inputs are NaN. This supports the observed provider behavior, not the injected reduction
seed. See [libdevice fminf](https://docs.nvidia.com/cuda/libdevice-users-guide/__nv_fminf.html).
Microsoft leaves operation order unspecified for the distinct WaveActiveMin API; that does not
establish a WaveMulti payload guarantee. See [WaveActiveMin](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/waveallmin).
The strongest local finding is order-independent: selecting only among NaNs cannot create infinity.
Exact mixed-input/zero order is a CUDA-helper compatibility target, clearly separated from that fact.

A six-execution replay loads retained, hash-verified PTX generated by accepted 214 and recorded in 215. It uses full-warp all-quiet/all-signaling NaN inputs on the same scalar helper. NVRTC passes both;
NVVM O0/O3 fail both with the same infinities. Thus the scalar defect predates 216; its introducing
revision is not bisected. No accepted raw file was overwritten. This supplementary replay is separate
from the 288 matrix executions and does not imply the complete aggregate matrix ran on older code.

The predeclared supported-defect stop now applies after the bounded FP32 family classification.
FP64 GPU/oracle extension is intentionally deferred. No independent arithmetic or prefix issue was
investigated. The helper/special-case audit contains only ignored research apparatus: integer word
classification/order, source algorithm simulation and buffer marshaling. No production fallback or
special case was introduced. The next implementation must audit typed recipe operation closure and
reuse existing helpers, with a final fixture failing before and passing after in all three modes.

GPU smoke is 4/4. All 17 accepted source hashes, 12 artifacts and 547 runtime input hashes match 216.
The registered ledger remains inherited: 1,653 cells, 1,600 correct, 53 failures and four resolved
histories. The 1,035 frozen results already inherited from full 214 remain historical, not fresh on
216 or 217. Units 478 plus one skip, toolkit 18 and six material compile/assembly cells inherit 216.
Latest full checkpoint is 214; implementation cadence remains one. No material runtime claim follows.
No GPU loss, system change, production build, push or newly registered corpus cell occurred.

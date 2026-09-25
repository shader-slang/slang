# Slice 219: establish FP64 masked min/max source behavior

## Motivation

The FP32 algorithm correction is accepted, but FP64 masked min/max remains intentionally rejected.
A width change must be supported by independent expected output, not inferred from the FP32 tests.
Consider the dynamically loaded scalar portion of the research kernel:

```slang
uint lane = cudaThreadIdx().x;
uint mask = uint(data[0]);
if ((mask & (1u << lane)) != 0)
{
    uint64_t bits = uint64_t(uint(data[32 + 8 * lane]))
        | (uint64_t(uint(data[33 + 8 * lane])) << 32);
    double value = bit_cast<double>(bits);
    double minimum = WaveMultiMin(value, uint4(mask, 0, 0, 0));
    uint64_t result = bit_cast<uint64_t>(minimum);
    data[288 + 2 * lane] = int(uint(result));
    data[289 + 2 * lane] = int(uint(result >> 32));
}
```

Distinct quiet/signaling payloads occupy both halves. Adjacent finite values differ below FP32
precision, and signed subnormals exercise the full 64-bit comparison domain.

## Proposed solution

Research establishes the source contract before a subsequent bounded admission slice. The concrete
CUDA algorithm is the same caller-seeded ordered comparison/selection and butterfly/scan distinction
validated for FP32. Extend the existing typed source recipe only after a final FP64 runtime fixture
reproduces rejection and then passes all modes. Keep FP64 prefixes and ordinary numeric min/max
outside that admission. No production change is part of this research.

The discovery manifest has reached its declared 100-identity maximum. The next slice should first
raise that explicit bound to 128 in a separate, tested infrastructure change, preserving its minimum,
old identities, source/oracle selection and duplicate/overlap checks. That runner-contract change
requires a full checkpoint. Only afterward register the FP64 fixture and implement admission.

## Change summary

Only the completed plan, this five-part report, semantic evidence and STATUS are durable changes.
Generated shader, 64-bit integer oracle, CUDA driver apparatus, NVRTC PTX/cubin, exact NVVM rejection
logs and all 112 output records live under `build/nvvm-loop/slice-219-semantics/`.

## Concepts and vocabulary

A _64-bit raw word_ is transported as low/high uint32 values, without converting through host floating
point. A _source contract_ here is the particular CUDA helper's selected original operand, not a new
portable promise about reduction order. _Admission_ means accepting a canonical typed helper during
NVVM preflight; this research leaves that boundary closed. A _capacity bound_ limits the explicit
discovery manifest before filtering and is separate from workload selection.

## Process report

Fresh-context delegation remains unavailable because of the app's agent-thread limit. The parent
uses WORKFLOW's local fallback and records that no independent worker review occurred.

`hlsl.meta.slang` creates the canonical scalar and Multiple helper signatures for double, double2
and double2x2. CUDA `WaveOpMin/Max` uses ordered comparison and selects its second operand on ties
or unordered input. `_waveReduceScalar/Multiple` starts from the caller. For low-bit contiguous
power-of-two masks it performs simultaneous XOR stages from population/2 down to one; otherwise it
scans original named-lane values in ascending order. Singletons preserve the original word.

The independent oracle classifies NaNs by the 64-bit exponent/fraction, treats both zeros as equal,
and compares other values through sign-aware integer ordering. It selects original integer words.
Hand checks cover negative/positive infinities, adjacent negative and positive finite values,
signed subnormals, equal signed zeros, unordered operand order, a signaling singleton, a two-lane
butterfly and a sparse ascending scan. No expected value comes from NVRTC output or host min/max.

The shader loads four double values per lane through integer words, then writes scalar, double2
and double2x2 min/max outputs as both halves. One 32-thread block runs per case. Every named lane
participates with the same mask. The apparatus also verifies input memory is unchanged and every
unnamed output retains its sentinel. The explicit masks are full, low16, high16, low15, high17,
even, odd and singleton31. Fourteen families cover finite values, infinities, signed zeros,
all-quiet/all-signaling/mixed NaNs, one quiet/signaling NaN at first/middle/last among finite inputs,
adjacent finite words and signed subnormals.

All **112 NVRTC O3 executions match exactly**. They compare 25,284 active double values (50,568 uint32
words) and 24,892 inactive double sentinels (49,784 uint32 words). The one emitted PTX assembles for
SM80. This is successful CUDA-helper evidence; no FP64 NVVM kernel executes in this slice.

Compiling the exact source at NVVM O0 and O3 returns 255 and creates no PTX. Both report:

```text
error[E52017]: direct NVVM lowering does not support Slang IR instruction or shape 'GenericAsm assembly=_waveMin($1.x, $0), signature=double(double, vector<uint,4>)'
```

That is the first unsupported helper, so these diagnostics do not independently establish the later
aggregate admission paths. `_getNVVMMaskedWaveScalarIdentity` deliberately rejects FP64 operations
other than ADD/MULTIPLY, while `_initializeNVVMMaskedWaveScalarOperation` currently enables the
source min/max algorithm for FP32 only. The source/IR shape is canonical, and existing double
comparison, select and two-word indexed-shuffle operations are already available. A future slice
must compose and validate them for each scalar/aggregate shape, preserving strict prefix rejection.
No signature reconstruction, source workaround or permissive fallback was tried here.

The helper/special-case inventory contains only ignored research apparatus: integer NaN/order
classification, source-algorithm simulation, and explicit buffer-half marshaling. No production
helper, guard, fallback or semantic representation changed. The source contract is concrete enough
to propose typed admission, while raw GPU validation of that direct path remains required.

Fresh smoke is 4/4. All 18 accepted source hashes, 12 artifacts and 548 runtime input hashes match
218 before and after research. Compiler SHA256 is
`a13354a47acefc8684cbfb5ae52b84bcf0331a83e5310e9675f53a0e030107d7`; provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36.
Native Ubuntu/L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86 and LLVM 14 are unchanged.

All registered results inherit 218: 1,656 cells, 1,603 correct, 53 failures and four resolved histories.
Its 1,035 frozen cells already inherited from full 214 remain historical, not freshly executed here.
Units 478 plus one skip, toolkit 18 and six material compile/assembly cells inherit unchanged artifacts.
Full checkpoint 214 remains latest; implementation cadence stays two. No new registered identity,
compiler build, production change, GPU loss, system change, material runtime claim or push occurred.

# Establish the CUDA clock helper contract

## Motivation

The frozen `slang-extension/realtime-clock.slang#cuda-1` workload still rejects at NVVM O0/O3
with `GenericAsm assembly=clock, signature=uint()`. Its result expression algebraically cancels
the clock values, so even a future pass would not prove live counter behavior. Consider instead:

```slang
uint2 first = getRealtimeClock();
uint low = getRealtimeClockLow();
uint2 last = getRealtimeClock();
```

Store all five words, repeat inside a dynamically bounded loop, and retain independently expected
integer work. The middle low-word observation must lie within the two surrounding counter reads,
allowing modular wrap. Distinct loop observations must advance over the bounded experiment. These
properties distinguish working clocks from commoned or hoisted calls without predicting ticks.

Clock support affects two measured frozen cells and has a narrow, reusable contract. BF16/FP8 and
resource work remain separate. Material was reconsidered, but its bindings, textures/LUT/input and
output oracle remain absent; six inherited compile/assembly passes are not runtime evidence.

## Proposed solution

This research recommends two narrowly typed, zero-operand provider operations for CUDA `clock`
and `clock64`, implemented with side-effecting inline PTX reads of `%clock` and `%clock64`.
The canonical source result types are unsigned 32-bit and signed 64-bit respectively. Preserve
the existing public helper's bit-preserving low/high-word conversion. Reject wrong arity/types
before provider loading. Do not change the frontend, source library or public timer meaning.

Do not directly map these operations to LLVM's clock intrinsics on this toolchain. LLVM 14 declares
both with `inaccessiblememonly nounwind` and explicitly describes nonconstant reads as unsuitable
for common-subexpression elimination. Nevertheless, CUDA 12.9 libNVVM accepts the exact declaration
and merges consecutive calls at both O0/O3. At O3 it also hoists loop reads. Side-effecting inline
PTX preserves the observations in the measured cases. Pure inline PTX happened to retain adjacent
reads too, but that observation supplies no effect contract and does not justify omitting
`sideeffect`. This recommendation is a provider semantic operation, not source-text interpretation
or a legacy-IR attribute rewrite.

CUDA's [time-function contract](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-c-programming-guide/index.html#time-function)
is a per-multiprocessor cycle counter. Elapsed ticks include scheduling effects, so they do not
measure only instructions executed by the calling thread. The [PTX clock contract](https://docs.nvidia.com/cuda/archive/12.9.1/parallel-thread-execution/index.html#special-registers-clock64)
states that the low 32 bits of `%clock64` are `%clock`; both wrap. Despite the Slang function name,
the CUDA target does not select `%globaltimer`. This slice establishes no cross-SM synchronization,
frequency, wall time, memory fence, or precise scheduling of arbitrary unrelated work.

## Change summary

Only this report, the completed plan, compact semantic evidence, STATUS and durable design notes
change. Production, provider ABI 36, source library, tests, corpus registrations and runners remain
unchanged. Raw scripts, source/IR/PTX, binaries, inputs, complete GPU buffers and logs are under
`build/nvvm-loop/slice-236-clock`. [Compact evidence](semantic-evidence.slice-236.json) preserves
per-probe outcomes and hashes, with an indexed raw artifact set.

| Fresh check                  | Result                                                                      |
| ---------------------------- | --------------------------------------------------------------------------- |
| Runtime smoke                | 4/4 before GPU research                                                     |
| Original frozen clock cells  | NVRTC correct; direct O0/O3 retain exact E52017                             |
| Clock source/control runtime | 36/36 launches satisfy all predicates                                       |
| Naive intrinsic countermodel | 6 zero-round passes, 18 nonzero-loop counterexamples                        |
| Word reconstruction          | 3/3 modes, 192 independently expected output words                          |
| PTX assembly                 | 20/20 artifacts                                                             |
| Compiler experiments         | 21 successes, 6 existing direct E52017, 4 malformed-layout setup rejections |
| Preservation                 | 31 source, 12 artifact and 556 registered-input hashes exact                |

Latest implementation/full acceptance remains 235: 1,680 cells, 1,635 correct, 45 unresolved and
twelve resolved histories. This research freshly replays only three of those cells; other full235
results and gates remain inherited. Targeted acceptance remains 233; implementation cadence zero.
No baseline reset, corpus addition, full-corpus claim or material runtime/performance claim.

## Concepts and vocabulary

- **Live counter read:** a new observation of a changing special register, even with no operands.
- **Commoned read:** two source observations replaced by one backend value; zero parameters do not
  make this valid for a clock.
- **Low-word bracket:** the unsigned modulo-2^32 distance from the first clock's low word to the
  middle sample does not exceed the modulo-2^64 distance between the surrounding full reads.
- **Side-effecting inline PTX:** LLVM inline assembly marked `sideeffect`, preserving observable
  reads through the backend. It is not a claim of a CUDA memory barrier.

## Process report

The canonical producer is the CUDA target switch in `hlsl.meta.slang`: `getRealtimeClockLow`
produces GenericAsm `clock` with `uint()`, and `__cudaCppGetRealtimeClock` produces `clock64` with
`int64_t()`. Both carry `NonUniformReturn`. `getRealtimeClock` casts that signed bit pattern to
unsigned for its high-word shift. These are intentionally valid source shapes, not malformed IR.
Direct NVVM's typed operation catalog lacks the two primitives. The provider owns their backend
effects; the existing active-mask inline-assembly operation demonstrates the required infrastructure.
No new general equivalence, syntax reconstruction, operand-graph search or fallback is needed.

Minimal public-low/public-pair/raw64 probes compile through NVRTC and reject at the exact GenericAsm
boundary in both direct modes. A small program linked to the installed LLVM 14 libraries prints
actual intrinsic declarations. Initial library links failed for unavailable `tinfo` and then missing
static dependencies; retained logs distinguish setup failures from semantic evidence. Four first
libNVVM IR probes omitted the required data layout and rejected before code generation. The corrected
provider layout makes all four accepted: exact LLVM attributes and conservative `nounwind`, each
at O0/O3. Every resulting PTX uses only one 32-bit and one 64-bit read for two observations of each.
The source NVRTC adjacent probe retains four observations. Inline PTX variants compile and assemble.

Runtime probes use one complete 32-thread block, twelve unchanged input buffers: loop bounds
0/2/5/16 crossed with three seed/addend patterns. Each buffer contains 96 input words and space
for sixteen rows of six words per lane. Each iteration stores first-low/high, middle-low,
last-low/high and unsigned affine work. There are five family/mode combinations: public Slang via
NVRTC, a raw intrinsic countermodel at O0/O3, and a raw side-effecting PTX control at O0/O3.
The source and controls share exact input buffers and predicates; they are distinct source forms,
not an assertion of identical generated instructions. All 60 launches preserve inputs, unused
sentinels and exact modulo-2^32 work. They cover 11,040 active tuples / 66,240 active output words,
with 184,320 total output words including inactive sentinels.

The 36 public/control launches pass all clock predicates. Every nonzero intrinsic run fails the
low-word bracket: 2,208 failed tuples at each optimization level. For example O0 lane zero reads
first64 = last64 = 31,807,425,335,810 while its middle-low sample is four ticks ahead of that value's
low word. O3 additionally fails loop progress for all 288 active lane/case combinations; PTX shows
the reads before the loop. The six zero-round countermodel cases are valid no-op checks, not clock
support. Research failures are retained as counterexamples, never converted into passing corpus
cells. No requirement is made that consecutive individual reads differ, nor are actual timestamps
compared between launches.

The bounded ordering oracle interprets a modulo-2^64 distance below half the period as forward;
these short kernels are not evidence for arbitrary-duration/preempted execution. The low-word
bracket accounts for 32-bit wrap mathematically; this run does not claim hardware wrap was observed.
A separate deterministic reconstruction kernel tests eight 64-bit patterns including zero, low-word
wrap, signed boundaries and all ones. All three compiler modes preserve the original two words;
this proves the existing split/casts need no clock-specific repair.

There are no production helpers/fallbacks/special cases to inventory in this research. The separate
acceptance script recomputes work with a closed-form affine recurrence, decodes every complete raw
buffer, checks all relational failures and exact selected-cell outcomes, and verifies current
source/artifact/input hashes against full235. Delegation was unavailable (`agent thread limit
reached`, only the root active), so this was the workflow's recorded local fallback; the checker
is independent code, not an independent-agent review. A first frozen selector incorrectly included
`tests/`; its empty-selection result is retained, then the exact immutable ID was replayed once.
No incomplete or setup-failed command contributes a pass.

Actual source revision/commit is `13badacf96791bca856c322108d12d9acd3dc22d`. Compiler SHA256 is
`ca34db1a349ae8716785032a0a3b01b3e6cf8455f3137e9358e9d1ad4eca63cf`; provider remains
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`. Native Ubuntu/L4 SM89,
driver 580.126.09, target SM80, CUDA12.9.2/NVRTC12.9.86, LLVM14 and optimized host configuration
are unchanged. No compiler build, GPU loss, system/driver change or push.

Next bounded action: admit exact typed clock shapes, add side-effecting provider operations with
appropriate ABI negotiation, prove rejection of invalid contracts, and add a registered runtime
fixture with deterministic expected invariant results. Replay the research's unchanged inputs and
predicates; never require identical clock outputs. Preserve the original frozen source. Provider
contract changes require a full frozen/discovery checkpoint and all six material support cells
before acceptance, even though implementation cadence is currently zero.

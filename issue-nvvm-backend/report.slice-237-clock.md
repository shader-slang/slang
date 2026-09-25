# Preserve live CUDA clock observations in NVVM

## Motivation

The frozen `slang-extension/realtime-clock.slang#cuda-1` test rejects at NVVM O0/O3 because the
CUDA target produces unsupported `clock` and `clock64` helpers. Its expression cancels the clock
values, so accepting that test alone would not demonstrate a working timer. Consider this code:

```slang
uint2 first = getRealtimeClock();
uint middle = getRealtimeClockLow();
uint2 last = getRealtimeClock();
```

The middle low-word observation must lie between the surrounding full reads, allowing wrap. A
bounded dynamic loop must retain new observations and independently expected integer work. Accepted
[research236](report.slice-236-clock.md) proves naive LLVM clock intrinsics are commoned at O0/O3
and hoisted at O3 through libNVVM12.9; these are semantic failures despite successful compilation.

Clock support addresses a measured two-cell gap with an established implementation contract.
Material was reconsidered, but bindings, textures/LUT/input and output oracle remain absent. This
slice makes no material runtime or performance claim.

## Proposed solution

Map only exact canonical GenericAsm `clock` uint() and `clock64` int64_t() to two new typed
provider operations. The provider emits `mov.u32 %clock` and `mov.u64 %clock64` using LLVM inline
assembly marked `sideeffect`. ABI37 negotiates this new contract and rejects older providers.
Existing frontend target selection, NonUniformReturn decorations and public word splitting remain
the semantic source of truth. No library/runner change or alternative timer meaning is introduced.

## Change summary

- `slang-nvvm-ir-builder-api.h` appends CLOCK/CLOCK64 operation values and increments ABI36 to37.
- `slang-nvvm-semantic-catalog.h` owns exactly two zero-operand scalar overloads: unsigned32 and
  signed64. Compiler preflight and provider descriptor validation share this catalog.
- `slang-emit-nvvm.cpp` adds exact spellings to the existing canonical helper resolver.
- `slang-llvm-nvvm.cpp` emits the provider operations with side-effecting inline PTX.
- Builder tests verify descriptor rejection and two distinct serialized reads in both LLVM dialects.
  The existing emitter negative matrix gains ten invalid shape cases and checks rejection before
  provider/library/module discovery.
- `nvvm-clock-observations.slang` adds dynamic 0/2/5/16-round checks with independently evaluated
  affine-work expectations and clock predicates. Discovery adds one identity; frozen is immutable.
- Completed plan, compact full checkpoint, census records, STATUS and design notes retain the
  evidence and existing failure histories. Raw logs and buffers remain under ignored build/.

All final-source gates pass. [Compact validation](runtime-validation.slice-237.json) records the
complete inventory and exact five-field comparison against full235.

| Fresh check                                 | Result                                                                        |
| ------------------------------------------- | ----------------------------------------------------------------------------- |
| Smoke / focused fixture                     | 4/4 before GPU suites; 3/3                                                    |
| Relevant units / toolkit / runner contracts | 481/481 plus one existing skip; 18/18; 6/6                                    |
| Frozen                                      | 452 identities / 1,356 cells; 1,343 correct                                   |
| Discovery                                   | 108 old identities / 324 cells plus fixture3; 297/327 correct                 |
| Material support                            | 6/6 compile/assembly only                                                     |
| Clock replay                                | 60/60 launches, 11,040 tuples, five assemblies                                |
| Preservation                                | 1,635 prior correct cells retained; only two old clock fixes; three additions |

Total 1,683 fresh cells yield 1,640 correct, 43 retained failures and 14 resolved histories. All
1,680 old cells were freshly executed; no inherited final-source passes are claimed. There are no
missing, extra or duplicate cells, lost support, changed oracles, unexpected five-field deltas or
baseline reset. The twelve older resolved histories remain exact, two new histories preserve their
whole prior failure record, and all 43 unresolved first-known/reproduction records remain intact.

Every gate and the final recheck agree on 35 source paths (all 31 old paths plus the API header,
semantic catalog, builder unit and fixture), 12 artifacts and 557 registered input hashes. All
556 old inputs and all 241 indexed research236 artifacts remain unchanged. The separate audit
checks 672 compact references, full failure-history preservation and every replay buffer. Raw
`build/nvvm-loop/slice-237-after` contains `audit.json`, `replay-audit.json`, `frozen-comparison.json`,
`implementation.patch`, per-gate identities and logs; before proof is under `slice-237-before`.
This is ready for parent acceptance; no worker commit has been made.

Tested source is base `cc0982c68365933cbd6b7885e94cc033d3eea566` plus the recorded patch.
Compiler-library SHA256 is `a89e9b370b03e62a62fe5f6becaab312399d5cec60bac6d75a53ff649a75c19f`;
provider SHA256 is `dafc5a557ce6f83d358c89956910af9761e352bb2f70f5efc6d5e7bc7f8a89ea`.
The native Ubuntu/L4 SM89/target SM80/driver580.126.09/CUDA12.9.2/NVRTC12.9.86/LLVM14 optimized
host environment is unchanged. ABI37 is the deliberate provider contract transition.

## Concepts and vocabulary

- **Live observation:** each execution reads the changing per-SM register; no-argument clocks are
  not pure functions.
- **Low-word bracket:** the modulo-2^32 offset of the middle sample does not exceed the
  modulo-2^64 span of surrounding full reads.
- **Sideeffect:** LLVM inline-assembly effect flag preventing elimination/commoning of observations;
  it does not promise a CUDA memory fence, synchronized lanes or a shared cross-SM timebase.
- **Typed operation catalog:** the shared compiler/provider list of valid semantic descriptors,
  separate from LLVM's signless integer representation.

## Process report

The CUDA branches in `hlsl.meta.slang` intentionally produce GenericAsm `clock` uint() and
`clock64` int64_t() helpers carrying NonUniformReturn. The public helper converts the signed64
bit pattern to uint2 using the existing unsigned shift. Research236 independently tests boundary
patterns, so rebuilding this representation or adding clock-specific split/cast logic would be
unjustified. The producer is correct; missing backend operations are the actual gap.

`_resolveNVVMGenericAsmValueOperation` uses the final assembly spelling and
`_resolveNVVMSemanticValueOperation` checks `_isCanonicalNVVMIntrinsicValueHelper`, the zero
parameter count, exact semantic result type and `NVVMSemantics::find`. Two table entries suffice;
no parser, helper-name inference, operand walk, custom equivalence, reconstruction or fallback is
needed. Wrong signedness/width, floats, vectors, extra parameters and executable instructions
beside GenericAsm remain rejected. These are invalid contracts, so adding coercions would hide
invalid input rather than implement clocks. Ten frontend-valid negative sources exercise these
boundaries through the existing fake-loader test, before any provider load/module creation.

The provider's existing value-operation dispatch validates the same descriptor and then calls
`_emitIntrinsic`. Only the two finite semantic cases are new. The function constructs an integer
return type and `InlineAsm::get(..., true)` with the exact `%clock` or `%clock64` instruction and
register constraint. LLVM clock intrinsic mapping was rejected by measured counterexamples:
research found 2,208 bracket failures at each optimization level and 288 O3 lane/case progress
failures. A convergence flag or memory clobber would introduce an unsupported stronger contract;
neither is needed for the established live-observation semantics.

The helper/fallback inventory contains no new production helpers or fallbacks. The two provider
operation cases survive the input-shape audit: canonical source producers create the exact valid
clock contracts; the backend owns their effects; the existing catalog owns validity. Removing the
spelling entries restores the before fixture's direct E52017 rejection, and omitting sideeffect
would abandon the documented effect contract even if a particular optimizer happened to retain
reads. The provider unit checks sideeffect serialization explicitly, while runtime replay checks
actual observations. ABI37 uses the existing exact-revision negotiation, preventing old providers
from interpreting appended operations without the new contract.

The readable runtime fixture was finalized and tested before production edits. It passed NVRTC
and rejected both direct modes on the accepted compiler. Its source and TEST_INPUT directives
remain unchanged. The fixed affine recurrence expectations were independently evaluated modulo
2^32; the dynamic loop bounds include zero rounds. Clock checks use the same bounded modular
order, low-word bracket and loop progress contract as research; they never expect specific ticks
or require consecutive individual reads to differ.

Research replay reads all twelve original input buffers without rewriting them. Public Slang
runs through NVRTC O3 and direct O0/O3, and unchanged side-effecting PTX controls run at O0/O3.
Each mode/family executes the same complete 32-thread buffers. A separate checker uses a closed-form
affine expression and decodes every output, unchanged input region and inactive sentinel. The O3
public PTX retains three loop reads (64/32/64); O0 calls the two clock helpers for those three
observations. Static move counts across helper bodies are not runtime observation counts. The
intrinsic countermodels remain immutable historical failures; they are never counted as support.
The bounded modulo-2^64 forward-distance predicate assumes these short experiments finish within
half a counter period. There is no hardware-wrap observation or arbitrary-duration scheduling claim.

The provider contract triggers a fresh full checkpoint despite cadence zero. Preservation compares
classification, return code, complete execution counts, diagnostic and canonical shape for every
old cell. Only the two original clock direct cells may transition; additions remain separate.
All existing unresolved first-known/reproduction records and twelve resolved histories are retained.
The six material cells establish compile/assembly support only. No driver change, reboot, push or
worker commit is authorized or performed.

Independent parent acceptance verified 672 unique compact references, 917 unique references including
research artifacts, all final source/artifact/input hashes, and exact old outcome/failure-history
preservation. It independently decoded all 60 replay buffers and verified the fixed fixture
arithmetic expectations. Full checkpoint 237 is accepted: 1,683 fresh cells, 1,640 correct, 43
retained failures, 14 resolved histories and zero implementation slices since the checkpoint.

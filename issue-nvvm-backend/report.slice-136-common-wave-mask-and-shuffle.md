# Slice 136 common wave-mask and shuffle report

## Motivation and measured family

The Slice 135 census identifies wave/reconvergence semantics as the largest remaining healthy-MVP
cluster at 31 workloads. Exact final-helper grouping shows that 18 first blockers share existing
revision-27 provider semantics:

| Canonical CUDA helper | Specialized contract represented | First blockers |
| --- | --- | ---: |
| `__ballot_sync($0, $1)` | `UInt32(UInt32, Bool)` | 5 |
| `_waveShuffleMultiple($0, $1, $2)` | selected 32-bit vector from `UInt32`, same vector, `Int32` | 10 |
| `_waveAllEqualMultiple($0, $1)` | `Bool(UInt32, selected 32-bit vector)` | 2 |
| `__popc(__ballot_sync($0, $1))` | `UInt32(UInt32, Bool)` | 1 |

These are not admitted by fixture name. CUDA target specialization produces one linked one-block
`IRFunc` whose only ordinary instruction is the final `IRGenericAsm`; exact assembly selects the
compound semantic and the specialized function signature supplies its complete typed contract.

## Producer and legalization boundary

`StmtLoweringVisitor::visitIntrinsicAsmStmt` creates the target-selected `IRGenericAsm`. Linking
and specialization leave the exact mask, vector, lane, and result types on the helper function.
`_resolveNVVMGenericAsmValueOperation` now recognizes the exact scalar ballot through the existing
catalog descriptor. `_resolveNVVMGenericAsmCompoundOperation` recognizes the three compound helper
spellings only after `_isCanonicalNVVMGenericAsmValueHelper` proves the complete one-block shape.

Each compound resolution builds a single ordered step table. Both preflight and emission consume
that same table:

- selected-vector read-lane-at extracts every component, invokes the established scalar
  `WAVE_READ_LANE_AT` descriptor with the unchanged mask and lane, and reconstructs the exact
  vector result;
- selected-vector all-equal extracts every component, invokes scalar `WAVE_MASK_ALL_EQUAL`, and
  combines the predicates with the existing typed Boolean `BIT_AND` descriptor;
- ballot population count invokes scalar `WAVE_MASK_BALLOT`, then the established UInt32
  `COUNT_BITS` descriptor.

Capability discovery records every step before provider module creation. Emission uses generic
sequential extraction and vector construction. The isolated LLVM 14 provider and builder ABI stay
at revision 27; no wave-specific callback, vector-provider overload, or compatibility surface is
added.

## Focused differential outcome

All 18 native CUDA references are correct. Direct O0 and O3 each report 12 correct workloads and
six deterministic preflight failures. The same 12 workload IDs are correct in both modes and
receive 24 direct regression lanes.

The six unpromoted workloads advance to later canonical blockers:

| Workloads | Later blocker | Count |
| --- | --- | ---: |
| masked broadcast/read/shuffle over `array<int2,2>` | Void/out-parameter aggregate shuffle ABI | 3 |
| unmasked read over `array<int2,2>` | unmasked Void/out-parameter aggregate shuffle ABI | 1 |
| masked active product | scalar masked wave reduction | 1 |
| masked divergence | scalar masked wave minimum | 1 |

This evidence keeps aggregate out-parameter transport and wave reductions separate from the
selected value recipe. No syntax reconstruction or broader signature fallback is introduced.

## Fixed-denominator coverage delta

The durable census remains 452 eligible workloads from 448 sources: 430 MVP and 22 extension.
Native CUDA/NVRTC O3 is correct for 449 and has three infrastructure failures.

| Mode | Correct | Runtime mismatch | Preflight | Provider | Compiles and launches |
| --- | ---: | ---: | ---: | ---: | ---: |
| Direct O0 | 276 | 8 | 163 | 5 | 284 |
| Direct O3 | 272 | 16 | 163 | 1 | 288 |

Compared with Slice 135, both direct modes gain the same 12 correct workload IDs and lose none of
the previously correct IDs. Among 427 healthy MVP references, O0 correctness is 275/427 (64.4%),
O3 is 270/427 (63.2%), and both-mode correctness is 267/427 (62.5%).

The healthy-MVP Pareto ordering is now:

| Root-cause cluster | Blocked workloads |
| --- | ---: |
| Helper ABI/type contract | 28 |
| Aggregate/pointer/layout transport | 23 |
| Wave/reconvergence semantics | 19 |
| Ordinary intrinsic semantics | 18 |
| Ordinary numeric/bit operation | 17 |
| Residual target marker/undefined value | 9 |
| Atomic/wave operation | 8 |

The selected wave/reconvergence cluster falls from 31 to 19. The four aggregate/out-parameter and
two reduction workloads remain in that cluster under their newly exposed first shapes.

## Representative workload and productionization gates

The three release-gate workloads remain correct through native CUDA, direct O0, and direct O3.
Median standalone compile time and generated PTX size are:

| Gate | NVRTC O3 | Direct O0 | Direct O3 |
| --- | ---: | ---: | ---: |
| Resource/aggregate/helper | 384.8 ms / 8,889 B | 272.1 ms / 6,102 B | 268.7 ms / 919 B |
| Parameter-block layout | 364.2 ms / 8,839 B | 246.3 ms / 917 B | 248.3 ms / 793 B |
| Shared control/barriers | 378.6 ms / 9,190 B | 247.4 ms / 1,940 B | 254.0 ms / 1,404 B |

CUDA 12.9 `ptxas` accepts every representative direct O3 module for SM70, SM80, and SM90. Runtime
comparison uses the local RTX 5090/SM120. CUDA 13 tooling and physical SM70/SM80/SM90 runtime
workers remain infrastructure gaps.

## Validation

- Release host and unchanged isolated LLVM 14 provider builds succeed with exact ABI 27
  negotiation.
- Focused fake-provider compilation observes two scalar Float32 shuffles, two scalar Int32
  all-equal operations, Boolean conjunction, ballot, and population count; a signed-mask near miss
  remains deterministic E52017 before provider discovery.
- The final 18-workload family rerun reports 12 correct and six preflight failures in both direct
  modes.
- All 36 promoted native/direct CUDA lanes pass.
- The selected NVVM regression prefix passes 405/405; this is regression evidence, not the
  coverage denominator.
- The fixed 452-row census records +12/+12 exact gains and zero old-correct regressions.
- Representative direct O3 PTX assembles for SM70, SM80, and SM90.

## Self-review and rejected alternatives

The new helpers survive for distinct reasons. `_isCanonicalNVVMGenericAsmValueHelper` centralizes
the already-required producer invariant for direct and compound helpers. The compound classifier
survives because exact final assembly plus specialized type is the canonical CUDA producer output.
The ordered step representation survives because one source of truth feeds both capability query
and emission. The Boolean fake-provider result check survives because the generic Boolean binary
family was already supported but the fake type predicate could not previously return such a value
from a helper.

Widening the LLVM provider to accept vector wave descriptors was rejected: revision 27 already
expresses scalar operations and vector structure. Treating `__activemask()` as ballot was rejected
because converged-mask semantics are distinct and the isolated LLVM 14 provider has no matching
NVVM intrinsic in its headers. Aggregate out-parameter shuffle and wave reductions were rejected
from this slice because their newly exposed canonical shapes require different reusable
invariants. Removing compound recognition restores all 18 initial E52017 first blockers; removing
one step causes exact preflight or emission failure in the focused test, demonstrating ownership at
this compiler legalization layer.

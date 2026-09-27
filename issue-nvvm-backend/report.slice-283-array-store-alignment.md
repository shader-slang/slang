# Conservative alignment corrects small nested-array stores

## Motivation

Research280's valid three-element nested record copies lose padding at NVVM O3. Gate282 showed correct
ordinary stores to genuinely unaligned destinations, while typed optnone helpers introduced costly
aggregate call arguments. This experiment changes only store alignment annotations on actually aligned
roots, preserving canonical values and authored signatures.

## Proposed solution

No production change is selected yet. In exact280 emitted LLVM, four whole-array stores change
align4→align1; a separate281 pointer-copy reproducer changes only two. All allocations, loads, scalar
stores and signatures stay unchanged. Both candidates pass NVVM O0/O3; their original O3 controls
reproduce wrong buffers14 and9234. Every correct output is `[0,123,0,456]`.

| Frozen gate                        | Result                                   |
| ---------------------------------- | ---------------------------------------- |
| Two originals, O0 / O3             | Two correct / two wrong-output controls  |
| Two annotation candidates, O0 / O3 | Four correct GPU cells                   |
| Candidate N17, O0 / O3             | Both compile-only successfully           |
| Candidate N65536, O0 / O3          | Compile-only success /120-second timeout |

Small correctness is demonstrated, but the general size gate fails under120seconds/4GiB. There is no
unmodified large-case baseline here, so the timeout cannot be attributed to the annotation change.

## Change summary

Only the completed plan, this report, [structured evidence](research-evidence.slice-283.json) and
navigation change. All12requested dispositions are retained:6correctGPU/2wrong controls/3compile-only/
1timeout. Raw frozen sources, exact diffs, replay, logs and PTX remain under
`build/nvvm-array-store-alignment283`. No compiler/provider/harness/corpus change or rebuild.
37runtime/8source/2config/576input/22pin identities remain exact279. Full279/targeted233/cadence0 and
1740corpus outcomes remain inherited; research280's production defects remain open.

## Concepts and vocabulary

Store alignment states a guaranteed property of the pointer. Supplying align1 for an align4 address is
conservative and does not change the actual address, canonical layout or intended value. An authored
pointer signature may still become aggregate parameters during downstream optimization; these are
separate observations.

## Process report

Both sources use Cell={i16,{i16,i32}}, with leaves0/4/8 and stride12. Exact source-difference assertions
permit only four or two whole-array alignment changes. Direct call traces and actual PTX local depots
show align4 roots with aligned offsets. Original O3 PTX writes child.first into2/14/26 using adjacent
16-bit stores; candidate byte stores preserve4/16/28. There is no malformed Slang producer or second
type representation to repair. The independent281 source/old/destination/self-assignment oracle also
passes, avoiding280's documented correlated-check limitation.

The authored pointer-copy LLVM instruction count remains96 as N changes. At O3, N3→N17 PTX grows
284→367instructions and caller code120→289. NVVM promotes the incoming pointer internally into three
12-byte parameters atN3 and a204-byte aggregate atN17, despite unchanged source-level pointer signatures.
Do not claim the downstream ABI is unchanged. N65536O0 completes with197PTXinstructions/7973bytes,
two byte-copy loops and2359296bytes of local depot storage; the module was not launched.

N65536O3 does not return before120seconds under the4GiB virtual-memory bound. Its process124 and absent
completed compiler result/PTX are recorded; no retry or extra variant extends this slice. The bound is
an experimental constraint, not proof of failure with all resources. A small correction alone cannot
establish general compile scalability, and a missing original-large control prevents a regression claim.

Fresh author, separate reused-context independent reviewer and root audit exact deltas, independent
buffers, actual alignment/offsets, caller growth and provenance. These are direct-vendor experiments,
not newly supported language or a material/performance result. Next compare the unmodified large case
under identical bounds, then decide whether the scale limit is inherited or introduced before choosing
production scope. Skip Slack and continue the authorized loop.

# NVVM current status

The explicit NVVM target and execution-register intrinsic slice is complete and accepted.
The maintainer authorized continuing through tagged primitive migration, compound CUDA-text
recognizer replacement and bounded wave optimization on 2026-09-29. Next: synchronization intrinsics.
Continue until those slices finish or a human decision is needed. Skip Slack; no push or system
changes. Plans, reports and raw artifacts stay ignored; update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records the current full correctness checkpoint.
[Accepted identity](accepted-identity.json) pins compiler/provider/modules/configuration and layout.
[Focused evidence](focused-evidence.json) retains qualifications and failure histories under their
actual compiler identities. Performance measurements have not been refreshed for this compiler.

| Evidence                                          | Accepted result                                                                                        |
| ------------------------------------------------- | ------------------------------------------------------------------------------------------------------ |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                                                      |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories; exact outcomes and inputs unchanged               |
| Frozen / discovery                                | 1,356 / 384 cells, all preserved                                                                       |
| Native units                                      | 1,131 identities: 1,118 pass, 13 skip; all 1,125 earlier identities/statuses preserved, six new passes |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                       |
| Surface host-readback matrix                      | 83 cases × 3 modes: 214 passes, 24 compile failures, 11 retained NVRTC rounding mismatches             |
| NVVM surface qualification                        | 81 supported cases pass in each mode; two dynamic-index negatives remain per mode                      |
| Target/register focused coverage                  | 9 units and 8 source/GPU cells pass; includes three launch modes and both link-override directions     |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                                        |
| Runner contracts                                  | 119 pass, 1 inherited skip; 83 surface oracle/ABI CPU case contracts pass                              |
| Earlier focused/static/material-runtime evidence  | Original identities retained; no new static, external ABI, numeric-sweep or material-runtime claim     |
| Last full / targeted / implementations since full | target-intrinsics / target-intrinsics / 0                                                              |

Compiler source: `429e7c4436044030945a9c646e966007cbe11027` plus patch
`f6c5548892d687362aabeb908fffda7d60937bca9b2cfc5acf252968a32de6e7`; version `2026.18.3-336-g429e7c443`.
Loaded compiler SHA256 `c6914473ad5567889d6e8cc08943d9fa8689e89a386f8003b9c51f4f69fa89de`; provider ABI44 SHA256
`32673ec63c4d04fd4ef23001e608d0f9144139c7ae726e26b89fa96b0bbfb904`.
Later commits do not identify rebuilt bytes. Qualification uses native Ubuntu24.04, L4 SM89,
driver580.126.09, CUDA12.9.2/NVRTC12.9.86, LLVM14 and SM80. Installed layout is `build/RelWithDebInfo`;
raw validation is under `build/nvvm-target-intrinsics/`.

The full checkpoint retains its exact execution identity. A later reviewed assertion-only repair
changed two unit-test files and the unit-test plugin; all 7,242 other checkpoint artifacts, compiler,
provider, modules, runners, configuration and all 310 embedded shader literals remained identical.
Original plugin/source bytes are preserved. Final native and focused gates use the new plugin and
execute all four runtime smoke identities without skips. Current identity and the explicit test-only
transition retain both provenances; earlier focused/static/material-runtime/performance evidence
keeps its original identity.

Direct PTX now selects `case nvvm`; CUDA source and NVRTC retain CUDA selection. Core execution
helpers and varying legalization directly call twelve scalar LLVM register intrinsics. Their four
semantic tags and CUDA-global recognizers are removed. Ordinary comma-separated `__intrinsic_asm`
arguments predate NVVM and remain supported. The temporary parenthesized tags remain only for
families awaiting migration. The next bounded plan covers barriers/fences with preserved effects.

## Boundaries and next action

- Migrate synchronization, bit/conversion, integer-bit and scalar-math families before replacing
  remaining compound wave/resource/atomic text recognizers. Direct libdevice math needs coherent
  selected-library signature validation before output-module creation; preserve existing math semantics.
- Packed/normalized surfaces, arbitrary user-helper resource provenance, dynamic component indices,
  three-channel transfers and general aliases remain outside physical surface legalization.
  Unannotated int4 requires matching 32-bit channels; signed8 mismatch controls remain failures.
- The 36 main gaps remain: three graphics-packed column-major mismatches and 33 infrastructure/
  preflight gaps. Focused NVRTC nested integer-array failures and vendor timeout histories remain
  separate. Do not repack original uploads or relabel these cells.
- Checked memory/address plans remain authoritative. A transforming local-storage pass stays
  deferred; recursive admission, dedicated resources and structured conversions remain architectural debt.
- Wave/interface optimization leads need fresh measurements after implementation. Existing compile,
  dispatch, material runtime, numeric sweep and external Half ABI claims retain their tested identities.
- Repeated dispatch of bare static state in `nvvm-copyable-kernel-context` remains unresolved.
  Resolve the intended initialization contract before selecting a producer or fixture fix.
- Relinking an already-compiled, requirement-free component with `linkWithOptions` can alias the
  input and retain cached output after option changes. This slice qualifies fresh distinct composites
  sharing one session; it does not fix that existing shared API boundary.
- CUDA source can emit unsupported `dim3 == uint3` for direct vector comparisons of `cudaBlockDim()`
  or `cudaGridDim()`. The new fixture checks all scalar coordinates with the same launch/count/sum
  oracle; the vector-comparison issue remains separately recorded.

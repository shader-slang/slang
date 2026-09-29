# NVVM current status

The explicit NVVM target/register and synchronization intrinsic slices are complete and accepted.
The maintainer authorized continuing through tagged primitive migration, compound CUDA-text
recognizer replacement and bounded wave optimization on 2026-09-29. Canonical bit reinterpretation
and Half conversion producers are next. Continue until those slices finish or a human decision is
needed. Skip Slack; no push or system changes. Plans, reports and raw artifacts stay ignored;
update current documents in place.

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
| Native units                                      | 1,133 identities: 1,120 pass, 13 skip; all 1,131 earlier identities/statuses preserved, two new passes |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                       |
| Surface host-readback matrix                      | 83 cases × 3 modes: 214 passes, 24 compile failures, 11 retained NVRTC rounding mismatches             |
| NVVM surface qualification                        | 81 supported cases pass in each mode; two dynamic-index negatives remain per mode                      |
| Synchronization focused coverage                  | 8 units and 17 source/GPU cells pass; includes cross-warp NVRTC O3/NVVM O0/O3 and Vulkan neighbors     |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                                        |
| Runner contracts                                  | 119 pass, 1 inherited skip; 83 surface oracle/ABI CPU case contracts pass                              |
| Earlier focused/static/material-runtime evidence  | Original identities retained; no new static, external ABI, numeric-sweep or material-runtime claim     |
| Last full / targeted / implementations since full | synchronization-intrinsics / synchronization-intrinsics / 0                                            |

Compiler source: `02d4ef85db4c1a873408636d92dd6d6a4ee8e795` plus patch
`d557c75359e5b312e80f8f9bfbf231d14893e5ec25818bb3dcc02353c5a774de`; version `2026.18.3-337-g02d4ef85d`.
Loaded compiler SHA256 `58d5ee2de9b8ff12bcc129332bee33d8e8671b0673160ec21e5fba13aa85e3d7`; provider ABI44 SHA256
`6b4244498fc2556c55df288609ad1a8baec8b0a987b73af6064d82fe07024ec5`.
Later commits do not identify rebuilt bytes. Qualification uses native Ubuntu24.04, L4 SM89,
driver580.126.09, CUDA12.9.2/NVRTC12.9.86, LLVM14 and SM80. Installed layout is `build/RelWithDebInfo`;
raw validation is under `build/nvvm-synchronization-intrinsics/`.

The full checkpoint and current native/focused gates use the same final compiler/provider/plugin
identity. All four runtime smoke units execute without skips. Earlier target-selection, static,
external ABI, numeric-sweep, material-runtime and performance evidence retains its original identity.
The inherited 83-case CPU surface prerequisite uses unchanged harness/Python bytes; its fresh
supplemental run was captured after this checkpoint began, as disclosed in accepted evidence.

Direct PTX now selects `case nvvm`; CUDA source and NVRTC retain CUDA selection. Core execution
helpers and varying legalization directly call twelve scalar LLVM register intrinsics. Their four
semantic tags and CUDA-global recognizers are removed. Ordinary comma-separated `__intrinsic_asm`
arguments predate NVVM and remain supported. The temporary parenthesized tags remain only for
families awaiting migration. Core synchronization now directly names the three void barrier/fence
intrinsics; seven source tags and their numbered provider dispatch are removed. Existing scopes,
effect handling and direct LLVM attributes are preserved. The cross-warp helper loop passes all
three modes with independent count/error/checksum 128/0/4544.

## Boundaries and next action

- Migrate bit/conversion, integer-bit and scalar-math families before replacing
  remaining compound wave/resource/atomic text recognizers. Direct libdevice math needs coherent
  selected-library signature validation before output-module creation; preserve existing math semantics.
- Barrier intrinsics retain convergence, but helper declarations do not propagate transitive LLVM
  convergence metadata. The bounded noinline cross-warp loop is qualified; arbitrary helper
  control-flow transformations remain outside that evidence. No miscompile was reproduced here.
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
  input and retain cached output after option changes. Target-selection evidence qualifies fresh distinct
  composites sharing one session; it does not fix that existing shared API boundary.
- CUDA source can emit unsupported `dim3 == uint3` for direct vector comparisons of `cudaBlockDim()`
  or `cudaGridDim()`. The execution-register fixture checks scalar coordinates with the same launch/count/sum
  oracle; the vector-comparison issue remains separately recorded.

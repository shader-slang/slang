# NVVM current status

The target/register, synchronization and canonical conversion slices, including the authorized
module version 34 compatibility boundary, are complete and accepted.
The maintainer requested stopping after this slice on 2026-09-29. The development loop is stopped;
resume only on an explicit request. Integer bit operations remain next, followed by scalar math,
remaining compound recognizers and bounded wave optimization; none of those slices has started.
Skip Slack; no push or system changes. Plans, reports and raw artifacts stay ignored;
update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records the current full correctness checkpoint.
[Accepted identity](accepted-identity.json) pins compiler/provider/modules/configuration and layout.
[Focused evidence](focused-evidence.json) retains qualifications and failure histories under their
actual compiler identities. Performance measurements have not been refreshed for this compiler.

| Evidence                                          | Accepted result                                                                                                 |
| ------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                                                               |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories; exact outcomes and inputs unchanged                        |
| Frozen / discovery                                | 1,356 / 384 cells, all preserved                                                                                |
| Native units                                      | 1134 identities: 1121 pass, 13 skip; all 1,133 earlier identities/statuses preserved, 1 new pass                |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                                |
| Surface host-readback matrix                      | 83 cases × 3 modes: 214 passes, 24 compile failures, 11 retained NVRTC rounding mismatches                      |
| NVVM surface qualification                        | 81 supported cases pass in each mode; two dynamic-index negatives remain per mode                               |
| Conversion focused coverage                       | 7 units and 22 source/GPU cells pass; three-mode independent conversion oracle and canonical IR producer checks |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                                                 |
| Runner contracts                                  | 119 pass, 1 inherited skip; 83 surface oracle/ABI CPU case contracts pass                                       |
| Module semantic-version boundary                  | Three existing isolated static units pass; min=max34, container format2                                         |
| Earlier focused/static/material-runtime evidence  | Original identities retained; earlier static suites, external ABI, numeric-sweep and material runtime not rerun |
| Last full / targeted / implementations since full | conversion-intrinsics / conversion-intrinsics / 0                                                               |

Compiler source: `bfad03686e9071f3e5368258dd22755ad4d27803` plus patch
`c9ee85205ff9966edaeeec78c65f4a65f942f8fa63a7d10756bbc2a760dfb531`; version `2026.18.3-338-gbfad03686`.
Loaded compiler SHA256 `4f3a41a994d96baed77849c3f18e563bee06989b5776c95be2dbf226eff39397`; provider ABI44 SHA256
`6b4244498fc2556c55df288609ad1a8baec8b0a987b73af6064d82fe07024ec5`.
Later commits do not identify rebuilt bytes. Qualification uses native Ubuntu24.04, L4 SM89,
driver580.126.09, CUDA12.9.2/NVRTC12.9.86, LLVM14 and SM80. Installed layout is `build/RelWithDebInfo`;
raw validation is under `build/nvvm-conversion-intrinsics/`.

The full checkpoint and current native/focused gates use the same final compiler/provider/plugin
identity. All four runtime smoke units execute without skips. All 17 earlier focused features, including target selection and synchronization, and earlier static,
external ABI, numeric-sweep, material-runtime and performance evidence retain their original identities.
The seven runner suites and 83-case surface CPU prerequisite use unchanged Python/harness bytes;
they completed before this slice's selected validation gates and full checkpoint. Failed build, producer-route and vector conversion
attempts remain recorded with their exact causes and resolutions. Integration checks after build5
and before the final checkpoint reproduce all 36 historical CUDA source/reflection controls. Three
archived version 32/33 modules reject with E00130 and no output at O0/O3. Rebuilt version 34 alias CUDA emits 80
at SM80; rebuilt version 34 alias and surface modules compile and assemble through NVVM at O0/O3. The
current runtime/source/configuration hashes match the final checkpoint. The old wrong-stage and
80-to-89 wrong-branch failures remain historical, resolved by explicit rejection and recompilation.
These controls use historical references, not an immediate synchronization compiler snapshot;
compatibility compilation is separate from GPU correctness. The isolated static executable and
configuration have separate recorded identities, and its three units do not increase native counts.

Direct PTX now selects `case nvvm`; CUDA source and NVRTC retain CUDA selection. Core execution
helpers and varying legalization directly call twelve scalar LLVM register intrinsics. Their four
semantic tags and CUDA-global recognizers are removed. Ordinary comma-separated `__intrinsic_asm`
arguments predate NVVM and remain supported. The temporary parenthesized tags remain only for
families awaiting migration. Core synchronization now directly names the three void barrier/fence
intrinsics; seven source tags and their numbered provider dispatch are removed. Existing scopes,
effect handling and direct LLVM attributes are preserved. The cross-warp helper loop passes all
three modes with independent count/error/checksum 128/0/4544 under its original tested identity.

Six scalar bit reinterpretation APIs and scalar/vector Half conversions now produce canonical
BitCast/FloatCast; fifteen source tags and two lowering names are removed. Vector CUDA and NVVM
arms share existing numeric constructors. CUDA emits per-lane casts; NVVM retains the checked
conversion plan and qualified RN-even provider conversion. Provider IDs35/38, Half ABI handling
and packed-Half/double-word recipes remain. The new live-input fixture checks all six bit directions,
heterogeneous widths2/3/4, special values and constant rounding boundaries at NVRTC O3/NVVM O0/O3.
The earlier unsupported vector API is now qualified on both routes. The unchanged neighboring
Half fixtures and all corpus/surface obligations preserve their original inputs and outcomes.

## Boundaries and next action

- Semantic module version is 34 only, for every backend. Recompile old user and separately supplied
  built-in/standard modules. Metadata inspection of old modules remains supported; speculative
  binary imports warn E00131 and may fall back to source. This deliberately rejects earlier
  numeric capability catalogs; it does not stabilize their identities for future changes.

- When explicitly resumed, migrate integer-bit and scalar-math families before replacing
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

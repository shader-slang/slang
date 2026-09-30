# NVVM current status

The target/register, synchronization, canonical conversion and integer-bit slices are complete.
The maintainer resumed the target/intrinsic migration on 2026-09-29; continue with selected-libdevice
signature validation and scalar math, then remaining compound recognizers and bounded wave optimization.
Accept and locally commit each bounded slice before proceeding. Skip Slack; no push or system changes.
Plans, reports and raw artifacts stay ignored; update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records the current full correctness checkpoint.
[Accepted identity](accepted-identity.json) pins compiler/provider/modules/configuration and layout.
[Focused evidence](focused-evidence.json) retains qualifications and failure histories under their
actual compiler identities. Performance measurements have not been refreshed for this compiler.

| Evidence                                          | Accepted result                                                                                                                       |
| ------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                                                                                     |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories; exact outcomes and inputs unchanged                                              |
| Frozen / discovery                                | 1,356 / 384 cells, all preserved                                                                                                      |
| Native units                                      | 1,140 identities: 1,128 pass, 12 skip; six new passes, one previously skipped integer unit now passes; other 1,133 statuses unchanged |
| Semantic regressions                              | 1,248 identities: 1,170 pass, 78 skip; unchanged                                                                                      |
| Surface host-readback matrix                      | 83 cases × 3 modes: 214 passes, 24 compile failures, 11 retained NVRTC rounding mismatches                                            |
| NVVM surface qualification                        | 81 supported cases pass in each mode; two dynamic-index negatives remain per mode                                                     |
| Integer focused coverage                          | 19 units and 29 source/GPU cells pass, including independent live-input integer oracles and canonical producer checks                 |
| Target-switch capability coverage                 | 106 pass: all 97 existing identities plus nine dead/live helper and layout controls                                                   |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                                                                       |
| Runner contracts                                  | 119 pass, one inherited skip; 83 surface oracle/ABI CPU case contracts pass                                                           |
| Module semantic-version boundary                  | Three isolated static units pass; min=max35, container format2                                                                        |
| Earlier focused/static/material-runtime evidence  | Original identities retained; earlier static suites, external ABI, numeric-sweep and material runtime not rerun                       |
| Last full / targeted / implementations since full | integer-intrinsics / integer-intrinsics / 0                                                                                           |

Compiler source: `b27e5a5fc3cf1a98834a75e9909ac91be250b406` plus patch
`c472c4379e7fccff7714c7035ce22169fe2a1e1b1d19ee4a91881e10422a7dea`; version `2026.18.3-339-gb27e5a5fc`.
Loaded compiler SHA256 `5c497cd1e43be3199fa2524b5de83ba9c2d218cf60c422474d1e381725c4432b`;
provider ABI 45 SHA256 `15df6f94bcdd6e82e7d37129fc1bae9bff230c118b5191521080c747e5dac9c1`.
Later commits do not identify rebuilt bytes.

Qualification uses native Ubuntu24.04, L4 SM89, driver580.126.09, CUDA12.9.2/NVRTC12.9.86,
LLVM14 and SM80. Installed layout is `build/RelWithDebInfo`; raw validation is under
`build/nvvm-integer-intrinsics/`. Source, configuration, runtime and layout identities stayed fixed
through final validation. The three isolated static units have separate executable/configuration
identity and do not increase native counts. Their first run regenerated the timestamp-keyed core
cache after relinking; the repeated passing run preserved every before/after hash.

All 18 earlier focused features retain their original identities. Current integer evidence covers
explicit named operands, LLVM registry signature/ImmArg checks, scalar widths 8/16/32/64, vectors 2/3/4,
signed high-bit and zero sentinels, and original 8-bit public count promotion. NVRTC 8/16 scan/reverse
still lack CUDA prelude helpers; those failures remain under their original tested identity.
Narrow count alone passes all three modes. New cases do not enlarge the frozen corpus or surfaces.

Direct PTX selects `case nvvm`; CUDA source and NVRTC retain CUDA selection. Core execution and
synchronization helpers directly name LLVM intrinsics, and conversions produce canonical IR casts.
Integer helpers now name ctpop/bitreverse/ctlz/cttz using the existing comma-separated
`__intrinsic_asm` operands. Six integer tags, four lowering names and four CUDA-text recognizers are
removed. Numeric provider operation IDs 45/48 remain for compound wave recipes; retired IDs 46/47 stay reserved. Parenthesized
semantic tags remain only for families awaiting migration.

Shared target specialization selects branches and removes unreachable helpers before diagnosing
remaining unavailable target switches. Its temporary layout root preserves the linker's host-held
layout across pruning without retaining unused resources through later DCE. CUDA/HLSL/SPIR-V layout
controls and the live E41011 control pass.

Four immutable version 34 libraries establish the compatibility break: retiring owned reverse/high
tags rejects those bodies while public/count-low controls still compile. Reader version 35 then
rejects all eight old O0/O3 imports with E00130 and no output. Nineteen successor phases cover fresh
version 35 libraries, metadata, compile/assembly and removed source-tag diagnostics. Named operand
replays confirm real LLVM calls; dynamic scan flags reject before output-module creation.

## Boundaries and next action

- Semantic module version is 35 only, for every backend. Recompile old user and separately supplied
  built-in/standard modules. Metadata inspection remains supported; speculative binary imports warn
  E00131 and may fall back to source. No historical capability or semantic-op remapping is promised.
- Next, validate direct libdevice names/signatures against the same selected library supplied to
  libNVVM, before output-module creation. Pilot `round` with preserved scalar semantics, then migrate
  the remaining scalar-math family before compound wave/resource/atomic text recognizers.
- Barrier intrinsics retain convergence, but helper declarations do not propagate transitive LLVM
  convergence metadata. The bounded noinline cross-warp loop is qualified under its original identity;
  arbitrary helper control-flow transformations remain outside that evidence.
- Packed/normalized surfaces, arbitrary user-helper resource provenance, dynamic component indices,
  three-channel transfers and general aliases remain outside physical surface legalization.
  Unannotated int4 requires matching 32-bit channels; signed8 mismatch controls remain failures.
- The 36 main gaps remain: three graphics-packed column-major mismatches and 33 infrastructure/
  preflight gaps. Focused NVRTC integer-array/narrow-bit failures and vendor timeout histories remain
  separate. Preserve original inputs and failure classifications.
- Checked memory/address plans remain authoritative. A transforming local-storage pass stays
  deferred; recursive admission, dedicated resources and structured conversions remain architectural debt.
- Wave/interface optimization leads need fresh measurements after implementation. Existing performance,
  material runtime, numeric sweep and external Half ABI claims retain their tested identities.
- Repeated dispatch of bare static state in `nvvm-copyable-kernel-context` remains unresolved.
  Resolve the intended initialization contract before selecting a producer or fixture fix.
- Relinking an already-compiled, requirement-free component with `linkWithOptions` can alias the input
  and retain cached output after option changes. Target-selection evidence uses fresh distinct
  composites sharing one session; it does not fix that existing API boundary.
- CUDA source can emit unsupported `dim3 == uint3` for direct vector comparisons of `cudaBlockDim()`
  or `cudaGridDim()`. Execution-register coverage checks scalar coordinates with the same launch oracle.

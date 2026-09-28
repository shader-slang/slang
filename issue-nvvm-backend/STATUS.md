# NVVM current status

The Monday results refresh and corpus code-quality comparison are complete. This finite request does not resume feature development;
the next implementation still awaits the maintainer's scope decision below. Earlier authorization
permits independently reviewed local commits, with regressions or maintainer decisions stopping
continuation. Skip Slack; no push or system changes. Keep working plans, reports and raw artifacts
ignored; update current documents in place.

Start with the [architecture](../docs/design/nvvm-backend.md),
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md) and [RESULTS](RESULTS.md).
[HISTORY](HISTORY.md) explains Git recovery of superseded documentation and evidence.

## Accepted state

[Accepted baseline](accepted-baseline.json) records the full corpus-dispatch harness checkpoint.
[Accepted identity](accepted-identity.json) pins compiler/provider/modules/configuration and layout.
[Focused evidence](focused-evidence.json) preserves qualifications and failure histories under their
actual compiler identities. Current compilation and device-event measurements now use the accepted
compiler bytes; earlier diagnostic experiments retain their original identities.

| Evidence                                          | Accepted result                                                                        |
| ------------------------------------------------- | -------------------------------------------------------------------------------------- |
| Cases / sources / mode cells                      | 580 / 576 / 1,740                                                                      |
| Main outcomes                                     | 1,704 correct; 36 unresolved; 21 resolved histories; all outcomes and inputs unchanged |
| Frozen / discovery                                | 1,356 / 384 cells                                                                      |
| Native units                                      | 1120 identities: 1107 pass, 13 skip; all identities unchanged                          |
| Semantic regressions                              | 1248 identities: 1170 pass, 78 skip; unchanged                                         |
| Earlier focused regressions (not rerun)           | 62 neighbor GPU/source cells, 32 shared units, 12 permanent GPU cells pass             |
| Earlier direct static units (not rerun)           | 5 pass, no skip; role/cache, classifier and address-plan contracts                     |
| Earlier exported Half ABI (not rerun)             | 4 separate PTX caller cells pass; 65,536 records and 64 uint32 guards each             |
| Runtime / toolkit / material compile and assembly | 4 / 18 / 6 pass                                                                        |
| Earlier material runtime (not rerun)              | 12 cells pass across default/filtering profiles; 65 active and 63 guard records each   |
| Runner contracts                                  | 97 pass, 1 inherited skip                                                              |
| Dispatch-profile harness                          | 17 contracts pass; 10 positive mode cells and 7 negative contracts                     |
| Original-input dispatch timing                    | 3,402 measured / 3,480 mode-round cells; 30,618 accepted samples                       |
| Last full / targeted / implementations since full | corpus-dispatch-performance / corpus-dispatch-performance / 0                          |

Compiler source: `3c250bff847674a72bb95c89b19841620c920d72` plus patch
`0e2ef2f0b68008d8dad4159c4449dd19a63a378dc094b0ab1055f4be1f40cc4d`; version `2026.18.3-329-g3c250bff8`.
Loaded compiler SHA256 `a38fbec6243e37886b67c0ba803c193d0178819752a2deda93ea154aa7b67654`; provider ABI42 SHA256
`af1661de02c02d67f1eab60724558d7ab32269112cfea5a0a95326795ba792c4`. Later Git commits do not identify rebuilt bytes.
Qualification uses native Ubuntu24.04, L4 SM89, driver580.126.09, CUDA12.9.2/NVRTC12.9.86,
LLVM14 and SM80. Installed layout is `build/RelWithDebInfo`. The profiling harness rebuild changes
only `librender-test-tool.so` and its debug sidecar in the accepted layout; the library SHA256 is
`dc48eac65c46d563630b48052650dafd199035879f635693be7bd84893a16113`.
Compiler, provider, modules, configurations and original corpus bytes are unchanged.

The fresh full checkpoint preserves all 1,740 exact outcomes and input identities. Native units,
semantic regressions, runtime, toolkit and material compile/assembly gates were rerun successfully.
Earlier focused GPU/static/ABI and material-runtime qualifications retain their original identities.
Current code-quality evidence and the expanded presentation are under
`build/nvvm-results/2026-09-28-corpus-code1/` (`presentation-final/` and `monday-nvvm-data.zip`).
The unchanged original-input timing evidence remains under
`build/nvvm-results/2026-09-28-corpus-device1/`.
Earlier compilation and synthetic material measurements remain unchanged under
`build/nvvm-results/2026-09-27-monday-refresh1/`.

Standalone release compilation covers 504 cases/500 sources, with 76 explicit exclusions and
9,072 measured samples plus 3,024 warmups. NVRTC O3/NVVM O3 geometric-mean time ratios are 1.47×
for 406 frozen cases and 1.41× for 98 discovery cases; NVVM O0 gives 1.51×/1.49×. Material O3
compilation remains close (eval 1.381/1.376 s; sample 1.401/1.473 s, NVRTC/NVVM). Large synthetic
material device-event ratios are 8.95× eval and 6.29× sample, pooling 18 samples per mode. These
are separate scopes: timed standalone PTX remains compile/assembly-qualified; synthetic material
and original-input corpus dispatches each have their own GPU protocol.

Original-input dispatch timing now covers 449/452 frozen and 117/128 discovery cases in all three
modes and both reversed rounds. Of these, 557 cases fall below the conservative 0.1 ms ratio cutoff;
nine qualify. Eight wave/min/max fixtures show NVVM O3 taking 1.76–2.42× the NVRTC O3 interval;
FP8 scalar transport is near parity at 1.03×. These intervals include parameter upload and host
enqueue gaps, and the shaders include correctness checks. They do not establish general shader
throughput. All 78 excluded mode-round cells are retained: 72 reproduce the 36 accepted gaps;
six expose the shared context repeatability issue below. See RESULTS for the full protocol.

Fresh original-input code capture covers all 580 cases and three modes: 1,704 qualified cells,
36 retained gaps, and 567 complete O3/O3 comparisons. Offline SM89 assembly finds 194 cases
with identical bytes in every named executable section, 0 additional normalized-PTX matches,
46 similar static profiles, 327 different profiles, and 13 incomplete comparisons.
Similarity is a triage heuristic, not performance proof. NVVM uses fewer/equal/more hardware
registers in 103/402/62 paired cases.
The narrow masked min/max slowdown coexists with smaller PTX and fewer offline registers:
eligible-mask fast paths and aggregate traversal differ. Floating min/max already uses trees,
but retains mode selection within loops and repeated component traversal. Interface-return dispatch
shows a separate tag/control-flow simplification opportunity (33→94 PTX, 40→104 SASS instructions);
its short fixture does not support a runtime regression claim. Static metrics use fresh captures
and offline ptxas, not historical timed PTX or recorded driver-JIT machine code. No new timings,
compiler changes or broader feature qualification are implied.

## Boundaries and next action

The code comparison prioritizes narrow integer wave fast paths, a collective lowering that shares
aggregate traversal and separates loop-invariant algorithm selection, and investigation of
post-inlining interface tag/payload simplification. Preserve floating operand-order semantics and
measure each bounded change before assigning causes to whole-fixture timings. No implementation
slice has started.

- Checked memory/address plans remain authoritative. A transforming local-storage pass is deferred
  because the reviewed rewrite would leave existing conversion responsibilities in place. Recursive
  admission, dedicated resources and structured conversions remain explicit architectural debt.
- Half2/3/4 helper parameters/results now transport integer lane bits while values, arithmetic,
  storage and type admission retain their roles. Parameter-only and result-only tests span every
  16-bit encoding per lane in two correlated families; this is not Cartesian coverage. The original
  effectful-call failure is resolved with unchanged source/oracle and retained before failures.
  Separate callers preserve the existing direct-NVVM PTX export ABI; CUDA-prelude interoperability
  and unspecified return padding are not qualified. No provider ABI change or extra cache is needed.
- The 36 main gaps remain: three original graphics-packed column-major mismatches and 33
  infrastructure/preflight gaps. Three focused NVRTC nested integer-array failures and two large
  vendor timeout histories remain separate. Do not repack original uploads or relabel these cells.
- Internal identity-record array parameters, compact matrix layout, legal vector source updates,
  synthetic-texture material eval/sample, mini-LUT and selected dielectric eval/PDF queries retain
  their tested boundaries in the feature matrix. Full applications, arbitrary inputs, sampling
  distributions and unmeasured performance remain open.
- Surface investigation found a separate format-contract mismatch. Unannotated int4 surface kernels
  assume native 32-bit channels; RGBA8Sint allocations are actually four bytes per texel. Independent
  host readback fails all five packed-format cells and passes all five matching RGBA32Sint controls.
  Historical shader self-check passes do not qualify physical packed texels. The original NVRTC
  component-source failure also remains open. No compiler or original corpus input was changed.
- Repeated dispatch exposes a shared initialization/fixture issue in `nvvm-copyable-kernel-context`:
  all three modes fail repeat equality in both rounds, although ordinary single-dispatch checks pass.
  Both routes emit bare `static State state;` as uninitialized private context storage; only explicit
  `flag` and `visits` initializers run. Host debugger capture confirms a later launch returns all
  32 failure sentinels. Resolve the intended bare-static initialization guarantee before choosing a
  shared producer fix or an explicit fixture initializer. Inputs/oracles and compiler remain unchanged.
- Correctness comes first for the next implementation; settle the context initialization contract.
  The longer wave/min/max cases now also provide a measured optimization target. Neither finding
  authorizes restarting the feature loop. Surface work still awaits the maintainer's format-scope
  choice: explicit static-format
  conversion (recommended foundation) or separate native-width component legalization. Static format
  support would still require annotations; it would not silently fix undecorated runtime bindings.
  Generic runtime formats need their own qualified design. Keep arbitrary C RequirePrelude rejected,
  and return to a material integration requirement after the selected bounded work.

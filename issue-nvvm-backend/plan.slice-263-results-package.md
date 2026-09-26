# Produce the refreshed NVVM results package and stop

This bounded ExecPlan follows `.agent/PLANS.md`; it is committed under the NVVM reporting exception.
The finite authority is consolidation261 -> master integration/full acceptance262 -> results263 ->
STOP. This plan does not resume general compiler development, authorize publishing or pushing, or
permit further optimization experiments. The lead owns acceptance, tracked writes and local commits.

## Purpose and Observable Result

Provide a reviewable Monday package with current correctness status, reproducible material compilation
measurements, fixed-subset assembled-code observations, shareable figures and fresh-session commands.
Every number comes from accepted262 or validated263 measurements; no
historical or synthetic maintenance result may stand in for a new measurement.

## Progress

- [x] 2026-09-26 Prepare package structure and source audit without builds, GPU work or measurements.
- [x] Parent accepts262 full checkpoint plus explicit serial PCH infrastructure closure after reviewing all deltas/gates.
- [x] Confirm tested revision, compiler/provider/cache, inputs, toolkit/driver/device and manifests.
- [x] Run isolated maintained material protocol, preserve every attempt, and generate report artifacts.
- [x] Run quality on fixed12/36 cells with accepted262 correctness binding; generate quality report.
- [x] Optionally run already-supported shared-session comparison, only if scoped by parent; keep distinct.
- [x] Fill compact narrative/evidence links and review all plotted metrics against structured rows.
- [x] Format/review/commit bounded263 package, update STATUS with explicit stop, then complete request.

## Surprises and Discoveries

The merged NVRTC driver supports `-pch` conditionally for NVRTC>=12.8 plus leading include source.
Its PCH marker is raw artifact diagnostics; the ordinary CLI diagnostic bridge forwards parsed
entries only. The harness stores all CLI logs but cannot promise that raw PCH state appears in them.
Missing markers mean unavailable observation, not disabled cache. See RESULTS.md and the final package README.

The upstream `--fmad=false` change applies to explicit Precise mode. Current material commands have
no `-fp-mode`; the default maps to downstream Default, which adds neither `--fmad=false` nor
`--use_fast_math`. Do not label the material comparison precise or attribute timing changes to that
conditional flag without a measured option trace. Preserve existing flags for comparability.

## Decision Log

- 2026-09-26: Use tracked `nvvm-results.py`/RESULTS.md; no additional slice-specific run scripts.
- 2026-09-26: Present paired O3 register allocation as quality observations; retain O0/full metrics.
- 2026-09-26: Fresh-process timing intentionally follows warmups and may share filesystem/toolkit
  state. PCH observation remains unavailable unless separate evidence proves actual state.
- 2026-09-26: Full262 ledger is prerequisite, not an outcome to be inferred from263 compilation.

## Outcomes and Retrospective

Completed at workspace 201cea6c9 with unchanged compiler 49593da72. Material132/66 and quality36/36
pass; shared66 requests/11 batches plus 6 references pass. Evaluation O3 is near parity; sampling
is 5.04% slower with NVVM. Register allocation is mostly equal on the fixed subset, with mixed resource
tradeoffs. Results, exact scopes and limitations are in results/2026-09-26/README.md and report263.
Accepted262 retains 39 gaps and a parallel PCH incident with explicit serial closure. Loop STOPPED.

## Context and Current Pipeline

Maintenance261 is committed7395e611498564070be690afaba9f01b7a6ba732. Integration262 is accepted with explicit serial infrastructure closure. Existing maintained commands wrap frozen/discovery/complex runners; material
reuses compile_command and slice257's2 opposite-order rounds,2 warmups,9 samples. Quality requires
an accepted-full compact record and identical source/compiler/provider bytes. Report uses Matplotlib.

## Scope and Non-Goals

Scope: measured compilation/assembly, representative simple-code observations, readable summaries,
figures and a reusable refresh handoff. No compiler/source fixture changes, cache-disabling experiments,
new benchmark script, material GPU runtime claim, performance optimization, full-corpus kernel timing,
push, publication, driver/toolkit installation or general-loop restart.

## Architecture and Invariants

All raw evidence uses a unique ignored directory. Preserve failed attempts and every warmup/sample.
Only accepted262 contains correctness authority. Material rows must account for 132 compiles and 66
assemblies under the fixed 6-cell protocol, with stable per-cell PTX/cubin bytes and complete phase
fields. Quality accounts for 36 single compiles/assemblies and links fixed sources to accepted runtime.
Report artifacts derive from validated measurements; don't edit plotted numbers or synthesize gaps.
Keep exact outcome histories and evidence identities, but avoid repeated raw-artifact indexes.

## Interfaces and Dependencies

Canonical commands: `issue-nvvm-backend/RESULTS.md`. Existing plotting environment:
`build/nvvm-results-tools/bin/python`; pin/record actual Matplotlib version. Exact run root and final
package destination are chosen once by parent before work. Use matching RelWithDebInfo/native tools,
SM80 and selected CUDA root unless parent explicitly documents an environment transition. Run no
build/profiler/other timing concurrently; suites stay sequential and respect four total CPU workers.

## Milestones

1. Accepted262 handoff: resolve all input/outcome/unit/semantic changes, preserve histories and record
   final binary/cache identities. Confirm no build process remains before timing.
2. Run `material`, then `report`; review complete inventory, opposite orders, within-cell artifact
   identity, per-round statistics, actual cache/math limitations and failed-attempt retention.
3. Run `quality --correctness ACCEPTED262`, then `report`; review exact named entries, null metrics,
   module scope for SASS/text, and fixed manifest options. Quality latency is not a benchmark chart.
4. Follow RESULTS.md to copy compact outputs and write the package README. Retain raw roots
   and provenance links; inspect the figures as rendered. Parent reviews final package and commits.
5. Update STATUS with package path, last accepted checkpoint262 and STOP; send only previously
   authorized completion notification through the parent. Do not choose slice264.

## Validation and Acceptance

- Successful maintained commands and report validation; any rejected/incomplete run blocks its claim.
- Complete sample counts, distinct cell IDs and mode coverage; verify plotted values from summary.json.
- Runtime ledger supplies correct/unresolved totals, accepted transitions and history; no count copying
  from260 unless explicitly labeled historical comparison.
- Source/binary/toolkit/device references match262; any change after acceptance requires resolution.
- Ratio claims use explicit numerator/denominator and paired same-run settings; historical unpaired
  sessions are context only. IQR is distribution spread, not confidence interval.
- Every unavailable measurement is stated; no raw PCH marker, no proof of reuse. Material contracts
  remain unprovided, so no material GPU speed claim.
- Fresh reader can execute refresh commands without ignored slice-specific scripts or chat history.

## Failure and Recovery

Keep the failed run root. Do not silently retry inside the sample series or drop an outlier. Diagnose
outside the measurement and use a new root for an explicitly recorded complete rerun if necessary.
If source/binaries change, reestablish affected correctness gates. If a metric is unavailable, label
it unavailable and preserve the diagnostic rather than substituting zero. If262 is blocked, keep263
pending and stop measurement work; draft package content remains useful but unaccepted.

## Artifacts and Hand-Off

Copy only the completed compact package, accepted262 reference,263 plan/five-part report and current
STATUS updates to tracked locations under parent ownership. Raw measurements/failed attempts remain
under build. Refresh commands and copy steps are in RESULTS.md. The final package contains README.md,
PRESENTATION.md, generated summaries/charts, package.json and shared-session.md/json; it requires no
ignored draft or conversation history to understand or refresh.

## Selected output locations

Raw integration gates: `build/nvvm-maintenance/integration-gates`. Full checkpoint and measured
outputs: `build/nvvm-results/2026-09-26-integration`. Reviewed package: `issue-nvvm-backend/results/2026-09-26`.
The accepted262 ledger is `issue-nvvm-backend/runtime-validation.slice-262.json`.

## Final review

Independent numerical and artifact review passed; material/quality PNGs were inspected. No benchmark
retry or sample removal occurred. Package and handoff are complete; local commit closes this slice.
Send the previously authorized completion DM, then stop. No next implementation slice is selected.

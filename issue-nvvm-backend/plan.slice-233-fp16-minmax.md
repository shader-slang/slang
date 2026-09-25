# Admit FP16 masked MIN/MAX with exact source seeds

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow requires the completed plan and report
in the slice commit. The worker owns checkout writes; the parent owns acceptance and commit.

## Purpose and Observable Result

Dynamic half scalar/vector2/vector4 masked MIN/MAX reductions and four prefixes, plus existing
matrix reduction leaves, execute correctly through direct NVVM O0/O3. Exclusive half seeds are
finite raw `0x7bff`/`0xfbff`; source comparison order and raw NaN/tie selection remain observable.

## Progress

- [x] 2026-09-25: Read workflow, accepted research 232, earlier floating recipe and native build skill.
- [x] 2026-09-25: Selected bounded admission/seed change and wrote this plan before implementation.
- [x] 2026-09-25: Final readable fixture passes NVRTC and rejects direct O0/O3 with E52017 before edits.
- [x] 2026-09-25: Two-function patch builds; smoke 4 first, focused 3 and all 8,868 saved launches pass.
- [x] 2026-09-25: Completed targeted preservation, self-review and durable report/status; parent acceptance completed.

## Surprises and Discoveries

Research 232 establishes finite seeds are not neutral for infinities. Existing source-order code
from 227 already owns separate transmitted and returned state; no algorithm replacement is needed.

The frozen selection resolves all four original direct prefix failures: 319 correct and two unchanged
quad-control `RequireMaximallyReconverges` preflight stops. The existing source is
`tests/hlsl-intrinsic/quad-control/quad-control-comp-functionality.slang`. No further investigation
of this independent blocker is included.

## Decision Log

- 2026-09-25: Rank FP16 MIN/MAX first: four concrete frozen failures, complete independent research,
  existing provider support and two-function scope. Matrix prefix capability and arithmetic need
  independent semantics/ownership. Material runtime lacks binding/texture/LUT/input/oracle; no
  speculative material changes or performance claim. Revisit ranking after actual fresh outcomes.
- 2026-09-25: Use closed-form registered fixture plus unchanged broad research inputs/expectations.
  Keep numeric half MIN/MAX catalog excluded. Do not modify accepted research helpers or binaries.

## Outcomes and Retrospective

Worker validation and independent parent acceptance are complete. Tested base is
`54ec1bfe5d92f5f7361ae85a7815ae1767e7a46a` on `nvvm-backend`. Only the two planned emitter functions
change. All four original prefix failures resolve with GPU proof. Cumulative acceptance is 1,677
cells / 1,630 correct / 47 unresolved / ten resolved histories, with 642 fresh and 1,035 inherited
full 229 cells. All other old five-field outcomes and 554 input hashes remain exact. Smoke 4, focused 3,
units 479 plus existing skip, toolkit 18, contracts 6, six material cells, 8,868 replay launches and
all negative boundaries pass. The seed/admission inventory retains two valid policy extensions,
no new helpers/fallbacks and no producer repair. No oracle/fixture revision after before proof,
production retry, full-checkpoint trigger, GPU loss or system change occurred. The quad diagnostic
is handed off without investigation; material runtime remains unavailable for lack of its contract.
Latest full 229 remains; accepted cadence is two.

## Context and Current Pipeline

For mask 3 and two positive infinities, `WaveMultiPrefixExclusiveMin(halfValue, members)` must
return +65504 in both lanes. `hlsl.meta.slang` creates canonical scalar/Multiple GenericAsm;
`_resolveNVVMMaskedWaveScalarOperation` and aggregate leaf resolution validate those signatures.
`_initializeNVVMMaskedWaveScalarOperation` selects the source compare/select recipe and
`_getNVVMMaskedWaveScalarIdentity` supplies raw seeds. `_emitNVVMMaskedWaveScalarValue` materializes
semantic-width constants and implements descending XOR reduction / ascending shuffle-up prefix
for low contiguous power-of-two masks, otherwise original-input scans. Provider half lane reads,
ordered comparisons and SELECT preserve raw operands. This is valid canonical checked data;
there is no producer defect or new representation to introduce.

## Scope and Non-Goals

Only half MIN/MAX admission and finite seeds in those two emitter functions; one registered fixture.
No provider/ABI, frontend, aggregate classifier, FP32 prefix, ordinary shuffle, arithmetic/bitwise,
matrix prefix capability, runner or shared lowering change. Stop at the next independent blocker.

## Architecture and Invariants

One identity authority. No new helper, fallback or duplicate operation mapping. Source ordered
comparisons choose the second operand for ties/NaNs. Exclusive returned seed is separate from
caller-seeded transmitted state. Existing scalar recipes apply unchanged to aggregate leaves.

## Interfaces and Dependencies

Native Ubuntu, matching RelWithDebInfo bins/libs, L4 SM89 targeting 80, CUDA 12.9.2/NVRTC 12.9.86,
LLVM 14 and provider ABI 36. Source `build/nvvm-loop/slice-203-env.sh`. Build command:
`CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
GPU suites sequential; at most four CPU workers and two unit servers.

## Milestones

1. Search coverage; finalize readable raw16 fixture and preserve its TEST_INPUT lines. Capture
   final source/fixture/compiler hashes; NVRTC must pass and both direct modes reject before edit.
2. Extend exact half MIN/MAX seed/admission, format explicit C++ path, build and run smoke 4 first.
3. Focused3; replay accepted 232 inputs/expected buffers unchanged across all modes: 8,868 launches,
   10,783,488 output words. Require 40 minimal direct compiles, 12 matrix capability rejects and six
   catalog checks (four admitted, two excluded). Retain source/raw identities and all failed attempts.
4. Units 479 plus existing Windows skip, toolkit 18, contracts 6, selected frozen 321 and all discovery 321,
   six material compile/assembly cells and meaningful excluded operation boundaries.
5. Exact five-field comparison; preserve all failure histories, finalize evidence/report/design/status.

## Validation and Acceptance

Affected domain is masked half MIN/MAX and existing wave neighbors. Frozen selection 208 has 107
identities/321 cells. All 106 old discovery plus fixture 107/321 yields 642 fresh cells; remaining 1,035
frozen cells explicitly inherit full 229. Baseline 231 totals 1,674/1,623 correct/51 failures/six resolved
histories; additions are three separate cells, cumulative 1,677. Four FP16 prefix failures may resolve
or advance: only GPU-correct cells count as fixes. Compare classification,return_code,complete
execution_counts,diagnostic,canonical_shape exactly. Preserve all 554 old input hashes, record new 555,
29 old source paths plus fixture, 12 artifacts, actual source_commit/source_revision. Historical
healthy denominators 427/72 stay fixed. Latest full 229; cadence one becomes two only after acceptance.
A provider/shared lowering/ABI/runner change, uncertain impact or regression requires full checkpoint
using explicit census 195 frozen inventory. Final fixture changes require exact original-emitter
rebuild drill and fresh before proof. No redundant successful unchanged runs.

## Failure and Recovery

Retain each attempt separately under ignored slice 233 before/after paths. Stop GPU dispatch on
device loss; no driver/system edits or reboot. Diagnose regressions or revert bounded patch;
never reset baseline or adjust expected outputs to hide failure. Parent receives any unresolved gate.

## Artifacts and Hand-Off

Raw logs/scripts/binaries under `build/nvvm-loop/slice-233-{before,after}`. Checked-in compact JSON,
cumulative censuses, addition record, completed five-part report and plan, durable design and STATUS.
Hash-address raw results/indexes. Return ownership explicitly; no worker commit/push.

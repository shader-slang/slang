# Slice 202: Preserve CUDA integer texture descriptor conversions

This ExecPlan follows `.agent/PLANS.md`. The maintainer authorized starting the autonomous NVVM
loop on 2026-09-24. Completed NVVM plans and reports are committed under the AGENTS.md exception;
raw logs and generated artifacts remain under ignored `build/`.

## Purpose and Observable Result

Establish exact current-host preservation evidence, then admit the canonical conversions between
UInt64 and already-supported read-only texture descriptor handles. A real bound floating-point or
integer texture must produce the independently expected texels through either conversion at
NVRTC O3 and NVVM O0/O3. Arbitrary 64-bit handle payloads must round-trip without truncation;
only valid bound handles are dereferenced. The full before/after comparison is complete and
preserves every historical pass; all required slice acceptance gates have passed.

## Progress

- [x] 2026-09-24: Read workflow, status, prior acceptance, and local build skill.
- [x] 2026-09-24: Confirm clean `nvvm-backend` at `2634d3b9dac628991fde9da0cfc966f3001951b8`;
      submodules match pins, native Linux, L4 SM89, driver 580.126.09, CUDA 12.9.2 targeting SM80.
- [x] 2026-09-24: Refresh Debug tools; all four runtime fixtures pass without skips.
- [x] 2026-09-24: Discovery completed 82 identities / 246 cells, preserving all 216 old-correct
      cells with no classification changes. Complex completed six cells with the same two NVRTC
      passes and four descriptor-conversion preflight stops. Toolkit checks passed 18/18 cells.
- [x] 2026-09-24: Frozen completed exactly 1,356 cells, with 449/438/438 correct at
      NVRTC O3/NVVM O0/O3. No classification changes or old-correct losses against slice 195 plus
      the accepted slice-201 overlay. Both corpora preserve all 1,541 old-correct runtime cells.
- [x] 2026-09-24: Compare exact cells, confirm all successful execution counts, and review
      historical failures. Selected unit gate passes 473/473 with its one Windows-only skip.
- [x] 2026-09-24: Select texture-descriptor conversion from the recorded candidate ranking;
      complete baseline reveals no new correctness priority.
- [x] 2026-09-24: Implement the selected texture-only conversions and permanent fixtures.
- [x] 2026-09-24: Focused checks pass 8/8, runtime gate 4/4, units 473/473 with one
      Windows-only skip, toolkit 18/18, and focused compile/assembly 6/6 commands.
- [x] 2026-09-24: Extend discovery target normalization after the explicit native-CUDA
      contract was rejected; all four runner regression tests pass. Frozen overlap remains enforced.
- [x] 2026-09-24: Reassess both material entries/modes; record uninitialized-value IR as the
      next independent blocker. Material source and provider ABI remain unchanged.
- [x] 2026-09-24: Discovery acceptance completed exactly 249 cells: all 246 old cells and
      their diagnostics unchanged; the new texture fixture passes all three modes.
- [x] 2026-09-24: Complete all 1,605 final runtime cells, preserving every old classification
      and diagnostic and all 1,541 old-correct cells; add three correct texture fixture cells.
- [x] 2026-09-24: Complete self-review, final-source hash verification, formatting, and durable
      acceptance records for the local slice commit.
- [x] 2026-09-24: Record the maintainer-requested stop after this commit; no slice 203 started.

## Surprises and Discoveries

The shell sandbox wrapper cannot initialize its loopback interface; authorized commands run via
sandbox escalation. This is a tool execution restriction, not a compiler or GPU failure.
The coverage prototype passes its NVRTC O3 runtime lane with a typed output oracle (four 1.0
values from an actual texture). Both NVVM modes reject `CastDescriptorHandleToUInt64`; IR retains
both conversion directions across noinline helpers. The first probe used raw hexadecimal output
with a float oracle; enabling the existing `-output-using-type` option corrected the harness
configuration without changing expected values.

The existing conversion helper admits resource/descriptor identity casts. Integer conversions are
canonical library operations but are not admitted. Buffer descriptors carry more than a scalar
handle, so a prospective integer conversion must use the existing texture classification.

## Decision Log

- 2026-09-24, maintainer: Finish and commit this slice (202), then stop the development loop.
  Record follow-up candidates in STATUS without starting another slice.

- 2026-09-24, Codex: Keep compiler source fixed throughout baseline collection. Use two corpus
  workers and bounded suites; preserve all historical IDs and unsupported outcomes.
- 2026-09-24, Codex: Defer feature selection until baseline comparison; inspect existing texture
  fixtures during the run without making speculative compiler changes.

## Outcomes and Retrospective

Accepted with recorded capability gaps. Full frozen acceptance contains exactly 452 identities /
1,356 cells with 449/438/438 correct in NVRTC O3/NVVM O0/O3. Discovery contains 83 identities /
249 cells with 73 correct in each mode. All 1,602 previous runtime classifications and diagnostics
are unchanged; all 1,541 prior correct cells remain correct. The three added cells pass, and all
61 pre-existing failing cells remain explicit in `runtime-validation.slice-202.json`.

Focused checks pass 8/8, runtime gate 4/4, units 473/473 with one Windows-only skip, toolkit 18/18,
focused compilation/assembly 6/6 commands, and runner contract checks 4/4. Six complex cells were
reassessed: two NVRTC passes, four direct stops now at `LoadFromUninitializedMemory`. The compiler,
provider, tests, and runner source hashes correspond to final acceptance. The capability ledger,
per-identity TSVs, runtime manifest, five-part report, and STATUS form the durable handoff.
The loop stops after the slice-202 commit at the maintainer's request; slice 203 is not started.

## Context and Current Pipeline

The material constructs `DescriptorHandle<Texture2D<T>>(uint64_t(texture_index))` then obtains the
resource using `getDescriptorFromHandle`. `hlsl.meta.slang` emits the canonical
`CastUInt64ToDescriptorHandle`; `_validateNVVMFunction` rejects it before provider emission.
Existing descriptor type lowering preserves the underlying resource representation.

## Scope and Non-Goals

First refresh the complete host baseline. Extend only the existing descriptor conversion resolver
and its preflight, selected-value validation, and emission switch cases. Add one runnable
texture/64-bit-boundary fixture to discovery and a diagnostic fixture that preserves rejection of
scalar casts for buffer descriptors. Reuse existing texture type classification and identity
emission. Preserve every existing corpus identity and expected output. Do not change the provider
ABI, supported texture families, buffer layout, material source, driver, or hardware.
No push, publication, reboot, or material runtime/performance claim is authorized by this plan.

## Architecture and Invariants

Correctness comes from actual executed output checks. Old passes remain obligations across hosts;
known failures remain visible. Compile/assembly-only complex cells do not establish GPU behavior.
Any accepted feature needs a failing-before/passing-after executable test at direct O0/O3.

## Interfaces and Dependencies

Use the configured native Debug tools, pinned LLVM14 provider ABI 35, CUDA 12.9.2, SM80 target,
and L4 SM89 device. Source `build/nvvm-setup/env.sh`. Durable baselines are slice-195 frozen and
discovery TSVs, with the slice-201 wave TSV overlay and slice-200 environment findings.

## Milestones

1. Complete and compare current-host corpora under `build/nvvm-loop/slice-202-before`.
2. Confirm the candidate ranking below against complete results before compiler edits.
3. Promote the validated prototype into `tests/cuda/nvvm-texture-descriptor-conversion.slang`,
   preserve negative buffer coverage, and add the new runnable source to discovery.
4. Extend `_getNVVMDescriptorHandleConversion` in `source/slang/slang-emit-nvvm.cpp`, routing both
   canonical integer opcodes through its existing three consumers. No new representation/helper.
5. Run focused tests, format final source, rebuild, and run all acceptance gates.
6. Update the capability ledger, per-cell evidence, five-part report, and STATUS; commit this
   accepted slice and stop as requested, without starting slice 203.

## Validation and Acceptance

Run the individual WORKFLOW.md native Linux commands with `NVVM_RUN` set to
`build/nvvm-loop/slice-202-before`, recording each exit code. Build and runtime gate have passed.
Require exactly 1,356 frozen cells, 246 discovery cells, and six complex compile/assembly cells.
Check each old-correct identity/mode and actual execution counts; investigate every loss.
Acceptance of a compiler change additionally requires all WORKFLOW section 5 gates after its last
relevant change. No baseline observation alone constitutes feature acceptance.

## Failure and Recovery

Stop dispatches on GPU/device loss and retain evidence. Investigate timeouts and incomplete rows.
Resolve or demonstrate pre-existing failures before accepting changes. If a recorded stopping
condition applies, update STATUS with the exact next action and leave the plan unaccepted.

## Artifacts and Hand-Off

Raw baseline commands, logs, results, source revision, source status, and submodule pins are under
`build/nvvm-loop/slice-202-before`. Preserve compact per-cell outcomes, comparison, provenance,
completed plan and five-part report with the accepted slice. Update STATUS at checkpoints.

## Coverage Prototype (before feature selection)

Hypothesis: render-test can bind a real CUDA texture to a typed descriptor field, allowing an
existing harness to exercise descriptor -> uint64 -> descriptor transport across noinline helpers.
Create only an ignored fixture under `build/nvvm-loop/descriptor-probe`. Run its NVRTC O3 lane
with `slang-test -test-dir build/nvvm-loop/descriptor-probe -disable-retries` and compile the same
source directly at NVVM O0/O3. Expected evidence is independently checked texture output under
NVRTC and the known descriptor-conversion preflight stop under NVVM. Inspect IR to ensure the
conversion was not folded away. Promote the fixture only after baseline-driven feature selection;
if binding fails, discard this fixture and record why a different existing harness is needed.

## Candidate Ranking and Input-Shape Audit

The full frozen comparison confirmed the selection of integer texture-descriptor
conversion: both material entries need it, its canonical IR and 64-bit provider representation
already exist, and the executable prototype proves the missing boundary with real texture inputs.
Wave rotation/shuffle transport would unlock two frozen workloads but has broader subgroup
semantics and does not address the current material blocker. Acceptance-checker automation remains
useful infrastructure; this run uses an explicit saved comparison script rather than expanding the
compiler feature slice. Discovery's 100-source cap is not reached by this one addition. The known
multisample NVRTC assertion predates this slice and is described in the slice-200 report; it remains
an explicit failure, not a passing runtime or a new direct-backend regression.

The producer is `DescriptorHandle<T>.__init(uint64_t)` / `uint64_t.__init(DescriptorHandle<T>)` in
`hlsl.meta.slang`. The IR retains `CastUInt64ToDescriptorHandle` and its inverse across ordinary
noinline helpers; no syntax reconstruction or source-text interpretation is needed. CUDA layout's
`GetDescriptorHandleLayout` gives the descriptor exactly its resource's layout.
`NVVMTypeLoweringContext::lowerType` maps selected read-only textures to an i64 and descriptor
handles to their resource type. Thus forwarding the operand is valid only for the selected texture
family and exact UInt64 input/output, using the existing classifier. Raw buffers instead carry a
pointer/count aggregate; admitting their casts merely by descriptor membership would be wrong.
The negative fixture proves those casts remain rejected. Combined texture/samplers and other
unsupported texture families remain outside the existing classifier.

Final helper/fallback inventory: the existing conversion helper and its three opcode consumers
survive review. There is no new compiler helper, fallback, or parallel type mapping. The shared
texture classifier remains authoritative. The corpus target-normalization change also survives: it
removes a redundant target-based restriction while preserving exact source-identity checks.
The concrete failing test is the ignored texture-handle-boundaries prototype at both O0 and O3,
which reports E52017 on `CastUInt64ToDescriptorHandle` while NVRTC checks four zero failure masks.
The implementation owns this valid canonical input because the backend has not yet admitted the
already-defined conversion; fixing the standard-library producer would discard valid semantics.

## Final-Source Acceptance Commands

Source `build/nvvm-setup/env.sh`. Use separate evidence directories for pre-change probes and final
source. After formatting and rebuilding with the WORKFLOW command:

- Run `slang-test -disable-retries -api cuda tests/cuda/nvvm-texture-descriptor-conversion` for all
  three positive runtime lanes, and the buffer diagnostic fixture for both conversion directions
  at O0/O3. Run existing CUDA descriptor source-emission coverage as a compatibility check.
- Run all four runtime gate fixtures, the selected 473-test unit prefix suite, and the 18-cell
  toolkit compile/assembly gate using the exact WORKFLOW options.
- Run full 452-identity frozen selection and the complete discovery manifest (83 identities after
  the addition) at all three modes. Use at most four workers per suite, bounded by 30m.
- Reassess all six complex cells and record the next exact blocker for both entries/modes. Do not
  change the shader or claim material execution. Timings are not performance evidence.
- Compare exact requested inventories, execution counts, and old-correct cells to the fresh host
  baseline and historical preservation obligations. Report the three added discovery cells apart
  from preservation. Investigate every new loss; no exception to full fresh runtime acceptance.

If the minimal compiler change does not pass the focused fixture, trace the new failure before
adding another case. If the material exposes an independent blocker, record it for a later slice.
Do not extend scope simply to make the whole material compile.

## Corpus Integration Discovery

The first post-change discovery invocation exited before creating any result: its target adapter
rejects a selected native `-cuda` directive even when that source is outside frozen v1. This old
proxy for frozen membership conflicts with explicit frozen selection and the workflow's runnable
coverage extension. Revise the bounded plan to normalize `-cuda` alongside other target selectors;
keep the actual frozen-source overlap and duplicate checks as the source of truth. Do not conceal
the failure by selecting a different/non-executable shader contract. Add a small runner regression
suite covering mode normalization, preserved selected input/oracle arguments, and real manifest
frozen-overlap/duplicate rejection. Rerun all discovery identities after the adapter change.
The concurrent frozen acceptance uses only the unchanged census runner and final compiler, so its
results remain fresh for their domain. Record the failed selection attempt separately.

Focused final compiler checks pass 8/8 (three runtime, four buffer-negative, one existing CUDA
source-emission test), runtime gate 4/4, and toolkit 18/18. Both material entries at direct O0/O3
now report `LoadFromUninitializedMemory`; trace that independent next blocker without fixing it.

## Final Review Notes

No source change followed the successful focused gates. The discovery-only adapter correction was
validated by four regression tests and a complete discovery replay; all 82 prior selected contract
records are exactly equal before/after. Complete frozen replay uses the unchanged census runner
and final compiler. No runtime evidence is inherited. Formatting was applied; unrelated rewrites
of historical design-ledger tables were removed, retaining the formatted new section.

Two optional debugger probes failed to resolve a pending source breakpoint. The saved IR and
source trace establish the next undefined-value shape, but the report deliberately does not claim
which occurrence preflight visits first. No speculative undefined-value fix or new slice was started.

# Compare the original large array store under the same compile bounds

This ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception. The loop is active;
skip Slack, no push or system changes. Root owns acceptance and documents; a fresh worker owns
ignored prototypes, with a separate read-only reviewer. Only one writer and one compiler process.

## Purpose and Observable Result

Determine whether slice283's N65536 O3 timeout also occurs with the original alignment4 stores.
This separates an inherited scaling limitation from a demonstrated annotation-specific regression.
A matching timeout does not prove identical causes. No production correction is selected here.

## Progress

- [x] 2026-09-27: Read WORKFLOW/STATUS; slice283 committed as8f84b1cc2 with clean checkout.
- [x] 2026-09-27: Accepted 37 runtime/8 source/2 configuration/576 input/22 pin identities verified; exactly two compile-only cells frozen.
- [x] 2026-09-27: Root and independent reviewer verify both exact source derivations and unchanged 120s/4GiB process bounds. Fresh author original_array284 authorized to run.
- [x] 2026-09-27: Original O0 compiles; O3 times out at 120 seconds/4 GiB without completed compiler return or PTX. Each ran once, no GPU.
- [x] 2026-09-27: PTX and both dispositions audited; before/after identities exact. Independent review and root strict record generation pass; completed evidence ready for formatting and local commit.

## Surprises and Discoveries

Original O3 hits the same experimental bound as the inherited annotation candidate. O0 keeps word-copy loops, while the candidate uses byte-copy loops; neither was GPU executed.

## Decision Log

- 2026-09-27, root: Add the missing original-large control before making a production decision.
  Do not retry the timed-out candidate or add arbitrary size cutoffs.

## Outcomes and Retrospective

One compile-only success and one timeout. The size gate already fails without the annotation change; matching failures do not establish identical causes. Next: bounded provider correction with a constructed/snapshot/phi/unaligned promotion gate and full checkpoint. No arbitrary cutoff or new ABI.

## Context and Current Pipeline

Accepted279 splits direct nested-struct stores in `_emitStorePreservingNestedStructLayout`, while
arrays remain whole stores. The canonical Cell={i16,{i16,i32}} has semantic offsets0/4/8 and stride12.
Original small NVVM O3 stores lose padding. Slice283 changes only two array-store alignments4 to1 in
prototype281's pointer-copy reproducer; its small outputs pass, but N65536 O3 times out120s/4GiB.
Original-large was not run. Authored signatures stay pointer-based, but NVVM may promote arguments.

## Scope and Non-Goals

Exactly original-pointer N65536 at NVVM O0 and O3, compile-only, no GPU launch. Derive from281's
baseline3 by changing capacity/count bound only, and independently prove equivalence to283's
large candidate with only its two store annotations restored to4. Retain inherited283 comparison
identity. No source/provider/runner/corpus implementation, build, candidate retry or performance claim.
Run both independent baseline cells even if one fails; each failure remains explicit.

## Architecture and Invariants

Keep canonical arrays, original pointer signatures, align4 allocas/loads/stores and oracle code.
Only N/count bound changes from the281 source. Authored compact LLVM does not establish compact
optimized PTX, bounded storage or GPU feasibility. Two timeouts establish only common gate failure.

## Interfaces and Dependencies

Raw root `build/nvvm-original-array-size284`; reuse283's direct libNVVM replay with frozen source and
runner hashes. CUDA12.9.2/NVRTC12.9.86, SM80 on native Linux; accepted279 compiler9e013b2c/provider
ABI42 a861b242. Verify37runtime/8source/2config/576main inputs/22pins before and after via the accepted
identity verifier. No loaded artifacts change; full279/targeted233/cadence0 stays inherited.

## Milestones

1. Freeze source, two-cell inventory/options and replay/process scripts. Reviewer verifies only the
   two restored store annotations differ from283, and bounds match120s/4GiB virtual memory.
2. Run `python3 build/nvvm-original-array-size284/run.py` serially, each child in an owned process
   group with120s timeout,4GiB limit, gate<=1800s. Keep compiler/process return separately.
3. Inspect completed PTX instructions/parameters/local bytes; compare inherited283 artifacts without
   reclassification. Verify identities; independent audit and root strict record validation.
4. Complete compact five-part report, structured outcome/provenance record and navigation; format
   changed files and make a local research commit.

## Validation and Acceptance

One outcome per cell with exact source/options hash, process/compile return, execution/pass/ignore
counts and diagnostic. Compile-only success is not shader correctness. Timeout cannot be successful
compilation. Check exact canonical source delta and all unchanged identities. No full gates needed
for research-only files; all1740 main outcomes remain inherited and optimized280 defects stay open.

## Failure and Recovery

No installed binaries are touched. Preserve each failed attempt without retry and stop each owned
process group at its bound. Missing artifacts remain missing. If controls fail the same gate, discuss
only inherited size-limit evidence; if original succeeds, treat candidate as a potential regression
requiring another bounded investigation. User stopping instructions take precedence.

## Artifacts and Hand-Off

Keep raw sources/process logs/PTX under the raw root. Commit this completed plan, report.slice-284-
original-array-size.md, research-evidence.slice-284.json and STATUS/HISTORY/HANDOFF updates. Continue
from recorded outcomes, never silently rerun completed cells.

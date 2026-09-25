# Preserve signedness in CUDA firstbithigh helpers

This ExecPlan follows `.agent/PLANS.md` and the NVVM completed-plan commit exception. Fresh workers
remain unavailable at the app's agent-thread limit; parent execution uses WORKFLOW's local fallback.

## Purpose and Observable Result

Correct unsigned 32-bit firstbithigh when bit 31 is set, while preserving signed-negative highest-zero-bit
behavior. Research 222 proves CUDA source gives the wrong unsigned result, while typed direct NVVM,
signed 32-bit and both 64-bit variants pass. Move the complement to the signed helper, add an independently
expected runtime regression, and preserve every existing runtime contract in a full checkpoint.

## Progress

- [x] 2026-09-25: Select bounded fix on clean research 222 base `409ab717d05f1bbabfa24f8df83ce67cf648b47b`.
- [x] Add dynamic scalar/vector signed/unsigned 32-bit/64-bit regression; capture before failure.
- [x] Move complement from U32 to I32, format/build matching optimized tools.
- [x] Run focused, research replay, units/smoke/toolkit and full frozen/discovery/material gates.
- [x] Review exact outcomes and unchanged old contracts, finalize evidence/report/STATUS and commit.

## Surprises and Discoveries

Research 222: CUDA source has 24 wrong unsigned 32-bit scalar/vector words across 18 executions; direct NVVM
passes all cases. CUDA prelude is byte-identical to full 220; the failure predates 221. Existing frozen
firstbithigh tests mostly exercise signed literals and lack dynamic unsigned sign-bit coverage.

## Decision Log

2026-09-25: Fix the shared source helper, not direct NVVM or front-end lowering. 32-bit CPU and 64-bit CUDA
already separate signed complement from the unsigned CLZ operation. Full checkpoint is mandatory for
this shared prelude change even though implementation cadence is only one since full 220. No corpus
failure is expected to change because old covered cases pass; new regression and research prove fix.

## Outcomes and Retrospective

Accepted locally on 2026-09-25. CUDA unsigned firstbithigh now preserves bit31 while signed32 and
all64 controls retain their behavior. Exact research replay passes all 6,912 words. Full checkpoint
has 1,662 fresh cells: 1,611 correct, 51 retained failures; all 1,659 old outcomes are exact and three
additions pass. Six resolved histories survive. Slice 223 resets implementation cadence to zero.

## Context and Current Pipeline

`firstbithigh<T>` emits `$P_firstbithigh($0)` with typed semantic metadata. CUDA source chooses
U32/I32/U64/I64; vector maps components to scalars. U32 wrongly casts to int32 and complements input.
I32 delegates to U32, depending on that signed behavior. Move precisely that complement to I32 before
its delegation; U32 retains zero sentinel and 31 minus CLZ. Canonical types are correct; prelude owns
this source intrinsic implementation. Typed direct provider remains unchanged and is a preservation
control rather than the fix target.

## Scope and Non-Goals

Two existing CUDA helper bodies, one new discovery regression, documentation and acceptance evidence.
No new helper, intrinsic operation, ABI/provider/front-end change, numeric expected-output change,
other bit intrinsic expansion, unrelated prelude cleanup or material runtime claim.

## Architecture and Invariants

Unsigned zero returns 0xffffffff; otherwise highest 1 bit index, including 31 for every top-bit-set uint32.
Signed-negative values complement before calling the unsigned helper, preserving highest 0 bit behavior.
Keep all old source/oracle contracts exact. New fixture loads raw words and precomputed expected
indices for scalar and two-component vectors; expected indices use independent Python integer
bit_length and bounded complement. Research 222 replay keeps all source/inputs/expectations exact.

## Interfaces and Dependencies

Native Ubuntu L4 SM89 target 80 CUDA 12.9.2/NVRTC 12.9.86 LLVM 14 provider ABI 36. Local slang-build skill,
matching RelWithDebInfo tools, source `build/nvvm-loop/slice-203-env.sh`, sequential suites/maximum four workers.
Before compiler 55bd12f280ee51def87219c767557e198cbd07b9b99f06d118a20209dfb46598; provider remains
ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372. Build generated prelude embedding.

## Milestones and Validation

1. Add `tests/cuda/nvvm-firstbithigh-signedness.slang`, independent runtime uint table and three modes.
   Retain NVRTC mismatch and NVVM O0/O3 pass before changing helper; save fixture/source/binary hashes.
2. Move complement, format explicit paths, rebuild slangc/slang-test/render-test/test-server through
   native releaseWithDebugInfo preset with four workers. Add one manifest identity separately counted.
3. Run the four-case GPU smoke, focused new fixture plus related typed units, exact
   research 222 replay (18 executions/6,912 words), NVVM/routing/reporter units, 18 toolkit cells, discovery contracts.
4. Existing 32-bit/64-bit firstbithigh are covered in the full matrix. Full frozen inventory of 452 identities/1,356 cells using `--workload-ids-from issue-nvvm-backend/census.slice-195.tsv`,
   discovery inventory of 102 identities/306 cells, six material compile/assembly cells. All 1,662 runtime cells fresh; preserve all
   1659 old outcomes across five fields; three additions should pass. Expected: 1,611 correct and 51 retained failures,
   six resolved histories unchanged. Compare to full 220 plus targeted 221 fixes/addition.

## Preservation and Acceptance

Shared prelude triggers full checkpoint; no inherited runtime rows. Every prior failure retains its
first-known record, reproduction and actual failed outcome. Only the separate research 222 mismatch
is resolved by this slice; do not count it as a registered fix. Accept 223 as latest full, reset cadence.
Material remains compile/assemble only; missing application bindings/oracle blocks runtime claims.

## Failure and Recovery

Regressions block acceptance; trace new differences before proceeding, fix responsible helper or
revert bounded change without resetting baseline. Stop on GPU/device loss, no driver changes/reboot.
Owned timeout-bounded suites only; no push. Preserve apparatus corrections as separate raw evidence.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-223-{before,after}`, independent research replay/source, PTX/cubins, gate
logs and hashes. Durable fixture/manifest, plan, five-part report, full result/census, design/STATUS.
Parent locally reviews helper inventory (no additions), signedness trace, exact field preservation,
final tested identities and every evidence reference before local commit and next slice selection.

2026-09-25 before proof: final new fixture passes both NVVM modes and fails NVRTC on the first lane
whose vector includes a top-bit-set unsigned input. Exact input/fixture/old artifact hashes retained.
Moved only the signed complement from U32 to I32; formatter touched no unrelated code. Optimized
build in progress; no acceptance claim until complete full validation and exact preservation review.

2026-09-25 final optimized small gates: focused 4/4 plus the existing Windows-only skip, smoke 4/4, research 18/18 with all 6912 words exact,
units 478/478 plus existing skip, toolkit 18/18 and discovery contracts 6/6 pass. The 24 research mismatches
are fixed on unchanged shader/input/expected arrays. Full frozen/discovery/material gates are running
sequentially. Final compiler SHA256 01e06def851b6228dea63d2bbb18cb4c3167ea89542d542623ea79e9d6f3258d;
provider ABI 36 unchanged. All 549 old runtime input hashes still match 221. Acceptance remains pending.

2026-09-25 full-checkpoint progress: frozen NVRTC and NVVM O0 each completed all 452 requested
identities; O3 is running. Structured combined results are written after the entire frozen suite,
so no exact-preservation acceptance is claimed yet. Discovery and material remain queued. Required
small gates already passed on final compiler; preserve their raw evidence and resume the owned run.

2026-09-25 frozen preservation review: all 1,356 fresh cells match full220 plus targeted221 across
classification, return code, complete execution counts, diagnostic and canonical shape. Inventory is
exact with no duplicates:1,335 correct,5infrastructure and16preflight. Discovery102identities is now
running; material remains queued. Full checkpoint acceptance is still pending.

2026-09-25 final acceptance: discovery306cells276correct30retained failures, material6/6. Parent
verified138evidence references,24tested sources,12artifacts,550runtime inputs. All549old inputs and
101old manifest rows are unchanged. The focused summary was corrected from assumed5passes to actual
4passes plus the existing Windows-only skip; checker rejected the initial count before acceptance.
No test rerun or source change was needed. All51first-known failures and6resolved histories retain
original evidence. Commit accepted full223, then bounded research224 on the generated KernelContext
pointer preflight affecting two frozen prefix min/max tests. No baseline reset, driver change or push.

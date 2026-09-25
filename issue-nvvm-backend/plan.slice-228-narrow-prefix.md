# Establish narrow-integer masked min/max prefix semantics

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer explicitly requires the completed plan
and report to be committed with this slice; generated probes and logs remain ignored under build/.

## Purpose and Observable Result

Determine the exact source semantics and direct admission boundary for signed/unsigned 8/16-bit
inclusive/exclusive min/max prefixes, prioritizing frozen int8 exclusive min/max rejection.
Deliver independently checked runtime evidence and a bounded implementation proposal, without
changing compiler code, corpus manifests, or registered outcomes.

## Progress

- [x] 2026-09-25 Read repository/build/workflow instructions and accepted 227 report.
- [x] 2026-09-25 Verify branch/base, 27 source/12 artifact/552 runtime-input hashes and smoke 4/4.
- [x] 2026-09-25 Trace source promotion, identities and canonical typed transport/rejection.
- [x] 2026-09-25 Execute dynamic scalar/vector source probes with independent integer expectations.
- [x] 2026-09-25 Check minimal direct O0/O3 signatures and complete evidence, report and STATUS.

## Surprises and Discoveries

The only rejected recipe boundary is the scalar identity width guard. Existing narrow catalog
MIN/MAX/read-lane/select contracts pass 16/16; provider transports narrow bits unchanged. CUDA
source uses signed char for int8. All source probes match an independent set-extrema oracle.
Sandbox shell fails RTM_NEWADDR; authorized escalated shell was required.

## Decision Log

2026-09-25: Research only; source agreement alone cannot establish correctness. Use independent
integer min/max folds and boundary identities. Exhaust 8-bit input values where feasible. No full
corpus run is warranted for an unchanged compiler with matching provenance.

2026-09-25: Add 224 small wide-input truncation launches after the primary 17,248 cases to
verify initial low-bit conversion explicitly. Host-only exhaustive pair checks remain distinct
from GPU value coverage. No plan scope or production boundary was expanded.

## Outcomes and Retrospective

Completed 17,472/17,472 runtime launches, 15,654,912 exact output words, 64 expected direct
rejections and ten runtime PTX assemblies. All 27 source/12 artifact/552 input hashes match before
and after; smoke 4/4. No compiler/corpus changes. See report and semantic evidence for exact
per-type/mode counts. Parent accepts and commits after worker returns checkout ownership.

## Context and Current Pipeline

Frozen prefix min/max tests stop at canonical GenericAsm
`_wavePrefixExclusiveMin/Max(($1).x, $0)`, signature `int8_t(int8_t, vector<uint,4>)`.
`hlsl.meta.slang` owns specialization, CUDA prelude owns source behavior, and
`slang-emit-nvvm.cpp` owns canonical recipe admission and typed lane exchange. Inspect exact
functions before proposing changes; do not compensate for malformed producer data.

## Scope and Non-Goals

Signed/unsigned 8/16-bit scalar and representative vectors, four prefix operations, dynamic
inputs and masks. Full/low power-of-two/irregular/high/sparse/singleton masks and signed boundary/
unsigned high-bit values are required. 32-bit controls are optional. No production changes,
manifest additions, matrix capability fixes or investigation of the next independent blocker.
No material runtime claim: application bindings, textures/LUTs/inputs and oracle are missing.

## Architecture and Invariants

The independently derived oracle uses mathematical signed/unsigned values, exact type extrema,
and prefix membership. Narrow conversion retains low bits and interprets signed values with
two's complement. Audit C++ promoted comparisons and return truncation explicitly; distinguish
lane transport from arithmetic. Retain input/expected/output hashes and inactive sentinels.

## Interfaces and Dependencies

Native Ubuntu; matching RelWithDebInfo compiler/provider ABI 36, CUDA 12.9.2, L4 SM89 target 80,
LLVM 14. Source inspected `build/nvvm-loop/slice-203-env.sh`; use existing probe CUDA driver helpers.
Research outputs go to `build/nvvm-loop/slice-228-narrow-prefix`; durable evidence goes to
`issue-nvvm-backend/semantic-evidence.slice-228.json`.

## Milestones

1. Identity and 4/4 runtime smoke gate before research GPU dispatch.
2. Static trace and bounded minimal direct probes, retaining emitted source/IR/logs.
3. Sequential GPU runs against independent expectations, exhaustive 8-bit values and 16-bit edges.
4. Verify unchanged source/binary/input hashes, record exact evidence and next bounded proposal.

## Validation and Acceptance

Run `extras/validate-nvvm-runtime.py --config RelWithDebInfo --cuda-path /usr/local/cuda-12.9
--architecture 80 --output build/nvvm-loop/slice-228-narrow-prefix/runtime` after sourcing env.
Research commands after the smoke gate:

```bash
source build/nvvm-loop/slice-203-env.sh
timeout --kill-after=30s 15m python3 build/nvvm-loop/slice-228-narrow-prefix/probe.py
timeout --kill-after=30s 2m python3 build/nvvm-loop/slice-228-narrow-prefix/truncation.py
python3 build/nvvm-loop/slice-228-narrow-prefix/summarize.py
```

Maximum four CPU workers; GPU suites sequential; bounded timeouts. Reuse exact 227 ledger:
1668 registered cells, 1617 correct, 51 failures, 6 resolved; 633 fresh at 227 and 1035 inherited 225.
Latest full 225 and implementation cadence 1 remain unchanged; discovery 104. Research executions
are separate from registered runtime cells. Require all intended probe cases and exact comparison.

## Failure and Recovery

Stop GPU work on device loss. Retain failed attempts separately. Unexpected source/oracle
mismatch requires isolated investigation before declaring a semantic result. Do not alter expected
results to hide failures. Code remains unchanged, so rollback is removal of generated research only.

## Artifacts and Hand-Off

Completed plan, five-part report, semantic evidence and STATUS; no commit/push by worker.
Parent owns acceptance and local commit after worker returns write ownership.

2026-09-25 parent acceptance: independently reviewed the mathematical oracle, complete launch
inventory/output hashes and exact rejection logs. Verified 187 evidence references, 27 source hashes,
12 artifact hashes and 552 unchanged registered inputs. Registered ledger/cadence remain unchanged.

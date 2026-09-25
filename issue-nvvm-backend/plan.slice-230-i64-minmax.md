# Establish 64-bit integer masked min/max semantics

This ExecPlan follows `.agent/PLANS.md`. The maintainer requires the completed plan/report to be
committed with this NVVM slice; raw research artifacts remain ignored under build/.

## Purpose and Observable Result

Establish an independent mathematical GPU oracle for signed/unsigned 64-bit masked reductions
and inclusive/exclusive prefixes, then stop at the research gate without compiler/provider changes.

## Progress

- [x] 2026-09-25 Read repository, workflow, status and local build skill; clean base ef0cd6bf92bced6c52245337f3abc500644ae6a9.
- [x] 2026-09-25 Capture all accepted identities and run smoke.
- [x] 2026-09-25 Trace source semantics and typed provider contracts.
- [x] 2026-09-25 Execute source GPU oracle and minimal direct rejection probes.
- [x] 2026-09-25 Complete evidence, report and STATUS; checkout ownership returns in the worker handoff.

- [x] 2026-09-25 Parent accepted the research after independent oracle/hash and evidence review.

## Surprises and Discoveries

The existing provider already transports both 64-bit integer halves exactly. Recipe identity admission
is the missing boundary. Naive admission widening would expose two shift-by-64 hazards in identity
construction. Existing Slang::bitCast supplies a defined bit-preserving conversion. No failed initial
experiment occurred; every expected direct rejection remains in its separate diagnostic log.

## Decision Log

2026-09-25: Research the coherent integer min/max family, including existing matrix reduction
leaves, before proposing a bounded implementation. Do not artificially gate reductions separately.

2026-09-25: Independent source/oracle and typed 64-bit controls pass, so propose only bounded emitter
identity admission/materialization next. Keep provider/ABI and canonical producer shapes unchanged.
Use 76 finite raw 64-bit patterns per type/mask; no exhaustive 64-bit claim. Inherit narrow/32 controls and all
registered slice 229 evidence because source/binary/input identities match; research does not advance cadence.

## Outcomes and Retrospective

Research gate passed: 2,128 source family launches and 6,384 typed 64-bit controls, with 14,981,120 exact output
words; 80 direct E52017/noPTX rejections; 24 matrix capability rejections; 8 descriptor contracts and
8 PTX assemblies; smoke 4/4. All 28 source/12 artifact/553 input hashes and submodule pins unchanged.
No production change or rebuild. Parent acceptance verified all evidence and independently
reconstructed every saved input and expected-output hash.

## Context and Current Pipeline

Frozen prefix failures reach canonical GenericAsm `_wavePrefixExclusiveMin/Max(($1).x, $0)`
with signature `int64_t(int64_t, vector<uint,4>)`. The CUDA source prelude defines value behavior;
NVVM recipes own admission. Slice 229 identities currently admit integer widths at most 32.

## Scope and Non-Goals

Both signednesses; scalar/vector2/vector4; all four prefix operations and both reductions;
matrix2x2 reductions only. Fourteen accepted masks, dynamic raw 64-bit inputs with independently varied
halves, signed/unsigned extrema and 32-bit boundaries. No exhaustive 64-bit value coverage claim, production edits,
corpus additions, matrix prefix capability fix, next independent blocker or material runtime claim.

## Architecture and Invariants

Use Python integer member-set extrema with explicit two's-complement interpretation, independent
of CUDA shuffle algorithms. Compare exact output words, inactive sentinels and unchanged inputs.
Inspect real typed 64-bit read-lane/MIN/MAX/SELECT and constant contracts; do not infer them from FP64.

## Interfaces and Dependencies

Native Ubuntu; matching RelWithDebInfo build; ABI 36; CUDA 12.9.2/NVRTC 12.9.86; LLVM 14; L4 SM89,
target SM80. Source `build/nvvm-loop/slice-203-env.sh`. No rebuild. Raw directory:
`build/nvvm-loop/slice-230-i64-minmax`.

## Milestones

1. Capture 28 source/12 artifact/553 input hashes and smoke 4.
2. Audit source promotion/overloads, provider lane transport and exact identity requirements.
3. NVRTC source-mode runtime proof, minimal O0/O3 E52017/noPTX with diagnostic/IR logs separate.
4. Check complete unique inventory; summarize source/input/expectation/output and raw hashes.

## Validation and Acceptance

Run `extras/validate-nvvm-runtime.py --config RelWithDebInfo --cuda-path /usr/local/cuda-12.9
--architecture 80 --output build/nvvm-loop/slice-230-i64-minmax/runtime`.
Executed `timeout --kill-after=30s 15m python3 build/nvvm-loop/slice-230-i64-minmax/probe.py`,
then the five-minute bounded `matrix-prefix.py`, C++ catalog assertions and `summarize.py` inventory
and identity audit. All raw commands/artifacts are hash-addressed in semantic evidence.
Run ignored probe scripts under bounded timeouts with sequential GPU suites, maximum four CPU
workers. Inherit all slice 229 registered runtime/units/toolkit/material evidence unchanged: 1,671 cells,
1,620 correct, 51 failures, six histories; frozen 452/discovery 105; full 229/cadence zero. Research executions
are separate; tested source is accepted ef0cd6bf, not stale prior result source_revision.

## Failure and Recovery

Stop GPU dispatch on device loss; no driver changes/reboot. Retain attempts separately. If source
and oracle disagree, isolate the discrepancy before any implementation proposal. Production remains
unchanged; no next feature begins in this slice.

## Artifacts and Hand-Off

Completed plan, five-part report, semantic-evidence.slice-230.json and STATUS; durable design facts
only if useful. Parent independently accepts and commits; worker does not commit or push.

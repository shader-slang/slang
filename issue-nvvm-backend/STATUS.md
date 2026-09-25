# NVVM backend status

Read [WORKFLOW.md](WORKFLOW.md) before resuming. The autonomous loop is authorized; the parent owns
independent acceptance and local commits. No push is authorized.

## Slice250 is accepted

The bounded BF16 negative canonical literal correction passed independent acceptance on provider
ABI 41. Read [plan250](plan.slice-250-bf16-signed-literals.md),
[report250](report.slice-250-bf16-signed-literals.md), and
[validation250](runtime-validation.slice-250.json).

The existing emitter now sends the checked uint16 BF16 encoding as its signed16 bit representation
to the integer-constant builder. `pack(BFloat16(-1.25f))` returns49056 (0xbfa0). Twelve exact values
cover both signs of1.25, zero, minimum/maximum subnormal, minimum normal and maximum finite.
Canonical producer, rounding/overflow, provider API, ABI 41 and BF16 storage/vector roles are unchanged.

Final source is accepted249 commit `c952717c4baa37cbe73d4eacdd5d011cce6a24c5` plus the250 diff:
119 source/generated/test snapshots,12 artifacts,563 runtime inputs. Compiler library SHA256
`ae6fe92965ed066a02ff9eab550093294b916c6b29f122f02e8a3f2ab8b74f60`; unchanged provider SHA256
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`.

- Fresh runtime 4, focused 18, units 513 with one unchanged skip, toolkit 18 and runner contracts6 pass.
  All513 prior unit pass identities are retained, including doubleSourceLiteralsRoundTrip.
- Full frozen 452/1356 preserves 1347 correct and 9 unresolved; discovery 115/345 preserves all 342
  old outcomes and adds 3 correct. Combined1701cells/1662correct/39unresolved. All1698 old outcomes
  match exactly in classification, return code, execution counts, diagnostic and canonical shape.
- All39 unresolved histories and 18 resolved histories survive. Texture/column-major wrong-output
  baselines are unchanged. No missing/extra/duplicate requested cells.
- Whole-buffer audit checks 3444 final words and 12 before words with zero mismatches. Final IR and
  PTX retain all 12 intended canonical literal signs and exact helper argument bits.
- Every material/backend/optimization cell is reassessed: complex 6 compile/assembly checks pass.
  No material runtime or performance claim; bindings/textures/LUT/input/output contract unavailable.

Latest accepted implementation and full checkpoint is250; latest targeted acceptance is233;
implementation cadence0. Rolling implementation history is246 material compile-time,249 FP8
scalar transport,250 BF16 correctness. Independent parent acceptance verifies every old outcome,
failure history, exact output, final IR/PTX, unit ID and identity, plus119 source snapshots and2132
indexed artifacts. Correctness priority explains this slice; reconsider material-driven work next.

## Next bounded action

Start a bounded research slice to refresh material compile-time profiling. Slice246 improved the AST
predicate; obtain a fresh profile before selecting another optimization. Do not infer the next
bottleneck from the old profile. All6 complex cells compile/assemble; absent material runtime contracts remain a limit on
performance/runtime claims. Do not expand BF16 vector storage, dynamic-object/aggregate ABI or FP8
runtime conversion merely to move an unsupported diagnostic.

The pre-existing BF16 constant failure recorded by249 is resolved by250. No next independent blocker
was exposed by the focused literal tests. Shared FP8 overflow still differs from CUDA SATFINITE;
research243 runtime conversion is not admitted. Known texture and column-major failures remain.

## Accepted historical evidence

[Accepted249](runtime-validation.slice-249.json) retains qualified FP8 scalar byte transport and
all-byte/cross-format/phi supplemental replays. Those supplements remain inherited with their249
identities; the full250 corpus and listed focused/gate results are fresh. [Research248](report.slice-248-texture-contract.md)
and [research247](report.slice-247-column-major.md) retain original failure histories and isolated
semantic controls. [Implementation246](runtime-validation.slice-246.json) retains the AST predicate
inlining and material timing evidence. [Implementation244](runtime-validation.slice-244.json)
retains finite/subnormal FP8 producer repairs; shared overflow policy remains separate.

## Environment and evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86,
LLVM14, providerABI41, matching RelWithDebInfo. Inspect/source `build/nvvm-loop/slice-203-env.sh` and
follow the local slang-build skill. At most4 CPU workers, sequential GPU suites and 30-minute bounds.

Raw roots `build/nvvm-loop/slice-250-before` and `slice-250-after` retain accepted249 before identity,
unchanged failing fixture, final snapshots, exact outputs, IR/PTX audits, commands and artifact index.
No GPU loss, driver/system change, reboot, push or worker commit occurred.

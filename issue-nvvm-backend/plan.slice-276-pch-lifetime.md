# Qualify NVRTC PCH namespace lifetime

This bounded ExecPlan follows `.agent/PLANS.md` and the committed NVVM plan exception. Authorized
loop active; Slack skipped, no push/install/host changes. Root owns tracked records and acceptance.
Fresh worker spawn hit thread limit; existing pch_repro275 may execute the bounded ignored harness.
Separate local audits replace unavailable independent review, without claiming independent review.

## Purpose and Observable Result

Research275 commit5afd52ef0 establishes shared-directory PCH ownership failure:8 native reuse
failures+2crashes among16 shared processes;24 serial/private controls pass. Before implementing a
private directory in `NVRTCDownstreamCompiler`, determine whether retiring an owner's directory
while NVRTC stays loaded breaks another owner's compile or later library cleanup. Deliver a bounded
lifecycle result and a concrete implementation gate, not a production fix or universal toolkit claim.

## Progress

- [x] 2026-09-26: Read275 evidence and source lifetime audit; accepted270 bytes unchanged.
- [x] Fresh276 worker unavailable at thread limit; existing worker/local lead audit fallback selected.
- [x] Pilot1 rc0; A/A/B statuses0/13/0,8077-byte identical PTX, all API/dlclose returns0;
      final PCH files empty. Root audits raw hashes and create/read/replace/unload ordering.
      Main20 approved after pilot, with frozen harness/inventory and cross-process PTX controls.
- [x] Frozen20 obligations/100 compiles pass; all named/empty lifecycle status sequences correct.
- [x] Root audits180 raw hashes,20/100 outcomes, handle order, same-source PTX and cleanup.
- [x] Full37/576/22/100/3 identity check exact, no residual owners. Compact record/report
      formatted and locally audited for commit; accepted270 unchanged.

## Context and Current Pipeline

Generated CUDA is a valid unnamed artifact; `ArtifactUtil::findPath` supplies its empty program name
to `nvrtcCreateProgram`. Automatic PCH then uses `default_program.pch`. Cache namespace belongs at
`NVRTCDownstreamCompiler::compile`, independent of program/source identity. NVRTC documents
`--pch-dir` but not safe directory retirement while the library remains loaded. Compiler wrappers
retain shared-library references; other references may outlive a compiler. `ScopeSharedLibrary`
unloads its own handle before releasing a scope, but cannot prove final unload across arbitrary
loaders. Existing temporary-file helpers reserve files, not atomic private directories.

## Scope, Architecture and Invariants

Research only: no production/build/config/input change and no GPU execution. Use installed
NVRTC12.9.86 directly through ctypes, canonical prelude and native-unit compute75/options. This models
owner lifetimes; it does not instantiate Slang compiler wrappers. Namespace is an explicit absolute
`--pch-dir`, never cwd mutation or synthetic source names. Empty and named program variants preserve
source identity. Destroy all program handles of a retired owner before removing its exclusively owned
directory. Never remove live-owner files. All temporary paths under ignored
`build/nvvm-pch-lifetime276`. Keep exact270 compiler/layout/input/pin/config identity.

## Milestones and Exact Execution

1. Inspect installed nvrtc.h declarations and275 pilot/native options. Prepare ignored ctypes child
   with typed API declarations and flushed JSONL stages (timestamp, PID, return codes, log, PCH status,
   PTX hash, program creation/destruction and dlopen/dlclose boundaries). Use a parent launcher with
   timeout180s and owned process group; strace -ff -ttt -s256 -e trace=%file. No external attach.
2. Qualify retained-directory pilot: canonical header source A, repeated A and changed-header source B
   with expected created/not-created/created. Require all compilation returns success and actual PCH
   files, correct API status meanings, deterministic PTX for identical source/options. Audit loaded
   NVRTC/builtins bytes and file trace. At most2 pilot revisions, retaining failed attempts. Root
   reviews before main. Model A/B exactly from native invalidation test, including valid header change.
3. Freeze20processes: five scenarios×two program names(empty,named.cu)×two repetitions, max1 at a time,
   no retries, total1800s. Scenarios: retained namespace baseline A/A/B; retire ownerA directory with
   library held then ownerB new directory A/A/B; retire then recreate same directory for ownerB;
   two warmed namespaces under held library, retire A then B A/A/B; stable namespace retained across
   owner destruction with extra external library reference and final release last. Keep exact launch
   order, commands, source/options/hashes and complete API outcomes. Explicitly model only the owned
   reference graph; no claim about arbitrary external callers/toolkits. Track same-source PTX equality
   to appropriate common control; do not expect changed source B to equal A.
4. Parse exact creation/reuse/invalidations and file cleanup per phase. Failure/crash/timeout/missing
   is never pass. A successful lifecycle does not establish documented guarantees across versions.
   Choose the smallest defensible production ownership policy or record the remaining contract gap;
   don't expand lifecycle scenarios post hoc to force a positive answer.
5. Verify unchanged37runtime artifacts/576inputs/22pins/100layout/3config and no owned processes.
   Root separately audits raw hashes/statuses/phase ordering and source/helper policy. Commit compact
   five-part report, completed plan, one structured record retaining20 obligations and pilot history.
   Update STATUS/HISTORY/HANDOFF, skip Slack, continue bounded production work if evidence supports it.

## Decision Log

- 2026-09-26, lead: Qualify lifetime before choosing adapter cleanup. Per-compile directories would
  discard reuse; PID-only naming can collide after process reuse; program renaming changes diagnostics.
  Private compiler-member ownership needs proof for still-loaded-library behavior.

## Surprises and Discoveries

Pilot confirms baseline created/reused/rebuilt states and benign cleanup ENOENT at final unload.
Existing shared-library owner scopes do not establish final library unload when
external references exist. No existing production private-directory RAII found in the audited paths.

## Validation, Recovery and Stopping Conditions

Own and bound every child; kill only owned process groups on timeout. Preserve all failures. Stop
this research inventory after20 children (or pilot gate failure); no stress-loop expansion. No full
correctness checkpoint because no accepted bytes change. A future shared-library production change
requires a full checkpoint, refreshed build version metadata and restored observer-object rebuilds.

## Outcomes and Retrospective

All20 processes/100 compiles pass; all PTX8077bytes hash0a1f8c6a01448db96e5e21b592a5d42490a83cd296cb1fddd1e1eab88aad2bcc.
Retirement with library held, same-path recreation and surviving-owner reuse succeed for empty/named
programs. No final PCH files. Select stable percompiler directory for separately qualified277.
Full270/targeted233/cadence0 remain inherited; original262 deletion signature remains open.

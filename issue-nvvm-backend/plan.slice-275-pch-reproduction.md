# Reproduce concurrent NVRTC automatic-PCH ownership

This bounded ExecPlan follows `.agent/PLANS.md` and the NVVM committed-plan exception. The authorized
loop remains active; Slack skipped, no push/install/host changes. Root owns acceptance/records/commit;
fresh worker pch_repro275 owns ignored harness/execution files. No compiler source/build changes.
Separate reviewer spawn hits the agent-thread limit. Root performs separate local method/evidence
audits under WORKFLOW's fallback; no independent-agent review is claimed.

## Purpose and Observable Result

Accepted262 records an NVRTC compile failing to delete `default_program.pch`; serial supplemental
passes preserve correctness but leave concurrent reliability open. Reproduce the failure with owned
processes and trace the file lifecycle. Distinguish cross-process directory sharing from ordinary
serial PCH creation/reuse. If a causal boundary is demonstrated, prepare a separately scoped fix;
otherwise retain the unresolved incident without claiming a closure from a few successful runs.

## Progress

- [x] 2026-09-26: Accepted274 committed ac05cced6; compiler/layout exactly270. Material inheritance
      repeated-work lead closed with no production change.
- [x] Read-only source audit traces normal emitted CUDA to an unnamed source artifact and PCH gate.
      strace is installed; actual owned-child tracing still needs qualification.
- [x] 2026-09-26: Worker verifies accepted identity and original incident hash. Owned-child strace
      preflight passes; native pilot exactly1pass/0ignored, observed3PCH creations/read reuse/replacements
      and cleanup. Root separately audits native argv/status and raw file trace.
- [x] 2026-09-26: Root approves fixed40process inventory with same native argv/options. Interleave:
      serial1,shared1,private1,shared2,private2,serial2,shared3,private3,shared4,private4,serial3,shared5,
      private5,shared6,private6,serial4,shared7,private7,shared8,private8. Freeze harness/inventory.
- [x] Executed all40 obligations; no missing, ignored, timeout or retry. All20 overlap checks audited.
- [x] Shared-path ownership/reuse failure established:8 assertion failures+2 crashes;24 controls pass.
- [x] Accepted state unchanged; root separate local audits pass. Compact report/record reviewed for commit.

## Context, Source and Contract

Original evidence is `runtime-validation.slice-262.json` validation_incidents[0], signature:
`error while deleting file "default_program.pch": No such file or directory`, in the frozen
`language-feature/dynamic-dispatch/layout-optional-field.slang#cuda-1` NVRTC O3 cell. Its source,
oracle, failed log/comparison and serial supplemental history remain unchanged. The log points to
prelude line8728 during compilation; it is not proof of a failure at library unload.

`TestToolUtil::setSessionDefaultPreludeFromExePath` installs a leading include of the canonical CUDA
prelude. `emitEntryPointsSourceFromIR` creates a blob-only unnamed source artifact; normal
`CodeGenContext::emitWithDownstreamForEntryPoints` retains it, unlike named pass-through sources.
`ArtifactUtil::findPath` then returns empty. `NVRTCDownstreamCompiler::compile` passes that name to
`nvrtcCreateProgram`, and adds `-pch` for NVRTC>=12.8 when the source begins with an include.
The adapter currently supplies no private PCH directory or PCH file-lifecycle policy.

[Official NVRTC12.9 documentation](https://docs.nvidia.com/cuda/archive/12.9.1/nvrtc/index.html#automatic-pch)
provides `--pch-dir` for automatic search/create location and describes cleanup at library unload.
Its automatic-PCH example uses an empty program name and produces `default_program.pch`. These
contracts motivate the hypothesis; they do not prove this incident's cause. Record installed
NVRTC12.9.86 bytes and exact options rather than assume another toolkit's behavior.

## Scope and Invariants

No production code/runner/corpus/option change. Keep37runtime artifacts,576inputs,22pins,100layout,
3config and frozen/discovery inventories exact. Use only ignored unique directories under
`build/nvvm-pch-reproduction275`; do not inspect/delete unrelated caches. Own at most2 concurrent
compiler processes, no competing build/GPU/performance work. CPU-only compile tests do not authorize
concurrent GPU execution. File tracing only for launched children, never attaching unrelated PIDs.
No environment/sysctl/capability/install changes. Accepted270 full correctness remains inherited.

## Milestones and Exact Execution

1. Read AGENTS/WORKFLOW/STATUS/RESULTS, report262/274 and existing
   `tools/slang-unit-test/unit-test-nvrtc-pch*.cpp`. Verify identities and original failure log hash.
   Record environment, actual compiler/provider/NVRTC libraries, source/prelude and tool versions.
2. Qualify owned-child strace with file syscalls (`-f`, timestamps/PIDs, file-related calls) and a
   trivial child, then one native `nvrtcPrecompiledHeaderInvalidation` unit invocation with retries
   disabled, from a fresh isolated cwd. Use absolute bin/test paths, `-skip-api-detection -api none
-use-shared-library`, so this CPU-only unit avoids GPU probes and test servers. Its default target
   is compute75; retain that actual scope separately from the original SM80 incident. The existing unit
   exercises probe/no-include/A/A/B through the Slang NVRTC adapter. Require an actual
   executed pass with zero ignore and trace-proven PCH creation/use/cleanup; exit0 alone is insufficient.
   Record all argv/env/cwd/loaded-library/file paths and native identities.
3. IMPORTANT: the unit probe can report Ignored when compile fails or the status marker is missing.
   Retain such a result as unqualified/failed observation, never a concurrency pass. If native tests
   cannot expose enough evidence or run in isolated cwd, propose one ignored tiny direct-NVRTC or
   public-Slang-API harness (native compiler only, no Slang rebuild). Capture exact API return/PCH
   status/log/PTX, stage timestamps and library lifetime. A direct-NVRTC reproducer needs a real
   Slang adapter control and cannot alone establish a fix for the original shader. Root reviews this
   fallback and its fixed inventory before execution; at most2 pilot revisions, all attempts retained.
4. After pilot qualification, proposed fixed inventory uses the same CPU-only workload sequence:
   4 rounds of2 sequential processes sharing a new directory (8processes),8 rounds of2 concurrent
   processes sharing a new directory (16processes),8 rounds of2 concurrent processes each with its
   own new directory (16processes), total40 process obligations. Fresh directories per round; same
   absolute source/header/options. Launch concurrency with a documented barrier or equivalent and
   retain overlap timestamps. Fixed order/interleave groups to reduce order bias; no automatic retries.
   Per-process180s, total1800s; max2processes. Trace all owned file operations. Directory isolation is
   an experimental control, not a claimed production fix. If a reviewed harness uses explicit
   `--pch-dir`, keep cwd common and compare shared versus child-private PCH dirs instead; predeclare
   exact commands/which single factor differs before main runs. No mixing post-hoc protocols.
5. Compare serial/shared/private outcomes, actual PCH activity and path ownership. Separate missing
   initial-cache probes from failed delete/create operations. Attribute failure to compilation versus
   teardown from native/API/log timestamps. Preserve PCH file snapshots when safely available after
   owners finish; never delete a live owner's file to manufacture failure. Failed/missing/ignored or
   timeout cells remain visible. If no failure occurs, state the bounded sample cannot close the old
   incident. No performance claim and no change of correctness baseline.
6. A useful positive result must connect the same filename/operation and independent owners to the
   observed error, survive serial/isolation controls, and match Slang's source/API behavior. Propose
   one next fix boundary preserving within-process reuse and diagnostic behavior. Audit existing
   temp-directory/loader lifetime helpers before proposing a new ownership abstraction. Do not
   silently disable PCH, add retries or serialize all compilation based on suspicion.
7. Verify unchanged source/config/artifact/input/pin/layout identities, no residual owned processes.
   Commit a compact five-part report, completed plan and one structured evidence record retaining
   every40process obligation (or explicitly reviewed fallback inventory), pilot failures, provenance,
   method limits and next gate. Raw traces/logs/harness/output files stay ignored. Format/review/local
   commit; update STATUS/HISTORY/HANDOFF, skip Slack and continue authorized loop.

## Decision Log

- 2026-09-26, lead: Return from material profiling to an observed reliability failure. Use the real
  adapter's existing CPU-only test first, requiring positive PCH activity and preserving its probe
  skip limitation. No compiler change until the lifecycle hypothesis is supported.
- No GPU correctness or concurrent-reliability closure follows from serial success or absent markers.

## Surprises and Discoveries

Native pilot passes while final cleanup includes two unlink ENOENT results. Therefore an ENOENT
syscall alone is not a failing compile or causal proof: require native pass/fail/ignore status and
file-lifecycle ordering, separating replacement from teardown.
The original failure is reported inside compilation diagnostics, although NVRTC also cleans automatic
PCH at unload. The invalidation unit's pre-check probe masks failure as Ignored; count execution/status
strictly and use raw file/API evidence. Fresh reviewer spawn again hits the thread limit;
the fresh275 worker starts successfully and root performs separate local audits. Exec sandbox bwrap fails before command launch, so authorized
escalated local commands are used, with no host-policy changes.

## Validation, Recovery and Stopping Conditions

Every launched child is bounded and owned; clean up only those children on timeout. Keep failed
attempts. No cleanup of shared preexisting caches. Stop this bounded investigation after the declared
inventory; unresolved evidence leads to a truthful negative result, not growing stress loops. A real
external resource or semantic-policy decision may require human input. No full checkpoint needed
because accepted code/binaries/inputs do not change.

## Outcomes and Retrospective

All40 obligations complete. Shared16:6pass/8native-fail/2SIGSEGV; serial8 and private16 all pass.
All shared rounds fail with actual PCH overlap. Original262 exact deletion signature remains open.
See research-evidence.slice-275.json and report.slice-275-pch-reproduction.md. Root verified320 raw
hashes/native outcomes/overlap and unchanged37/576/22/100/3 identity sets. Full270/targeted233/cadence0
remain inherited. No production change. Continue bounded276 namespace/lifetime gate before fix;
skip Slack, no push. Reviewed local commit closes275.

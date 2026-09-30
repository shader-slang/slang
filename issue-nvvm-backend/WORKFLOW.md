# NVVM development workflow

Start with [STATUS](STATUS.md), the [architecture](../docs/design/nvvm-backend.md), and the
[feature matrix](../docs/design/nvvm-backend-capability-ledger.md). Use [RESULTS](RESULTS.md) for
commands and [AGENTS](../AGENTS.md) for compiler methodology. [HISTORY](HISTORY.md) explains archive
recovery; current design must be understandable without reading completed slice narratives.

## Authority and ownership

The maintainer explicitly resumed development on 2026-09-30 with a faster migration workflow.
This supersedes the earlier stop-after-log request, four-worker limit, per-family numerical
campaigns, routine module-version bumps and automatic full-checkpoint cadence. The immediate
objective is to remove NVVM dependence on CUDA target-switch strings: express operations in the
core module through explicit NVVM branches, named LLVM/NVVM/libdevice calls and ordinary Slang
composition. Migrate related operations in substantial batches. Keep ordinary comma-separated
`__intrinsic_asm` arguments; remove the NVVM semantic-tag extension after its last consumers migrate.

Continue through bounded, reviewed local commits until the migration is complete or a concrete
regression/design decision needs maintainer input. No push, publication, system installation,
driver change or reboot is implied. Skip Slack notifications. The already-running log-family full
checkpoint may finish and be reused; do not repeat it under the new workflow.

After completion of the text-route migration, the maintainer explicitly resumed on 2026-09-30:
fix signed16 O3 correctness, run one consolidated integration checkpoint, then consolidate operation
dispatch/fakes, role-specific type admission and one complete resource/address/storage planning
family, reduce fake maintenance, refresh architecture, and address Float16 instruction selection.
STATUS holds the ordered work and semantic questions. Continue this authorized sequence in bounded
reviewed commits; stop only when human judgment is actually required. This supersedes the migration
completion stopping condition above. CUDA C++ is comparison evidence, not the universal oracle;
verify Slang semantics against Vulkan/D3D12 contracts and document intentional differences.

After this entire queued cleanup and Float16 sequence, the maintainer additionally authorized one
full NVVM validation checkpoint, correction of issues it reveals, and then resumption of feature
development under this standard workflow. This is an explicit integration milestone; it does not
restore full campaigns after every small change. Preserve exact failures, rerun affected checks
after fixes, and complete the checkpoint before returning to bounded reviewed feature batches.
Continue until a recorded stopping condition actually requires human input.

Use one implementation owner and an independent reviewer when available. The lead owns scope,
acceptance and commits. Read-only analysis may overlap; serialize builds and GPU runs. Use **eight
build jobs** on the upgraded eight-CPU host. Use focused test selections and existing runners;
do not build a new orchestration, approval or evidence framework for each batch.

## Select and implement

Read STATUS and inspect the working tree and actual compiler/provider identity. Preserve unrelated
user files. Follow the platform-specific slang-build skill and compiler methodology in AGENTS.
For substantial batches, keep a short uncommitted ExecPlan with scope, input-shape audit and actual
validation results. Do not turn the plan into the program backlog.

Inventory the remaining tags and CUDA-text recognizers and group migrations by shared lowering
and calling convention. Prefer a large batch of straightforward named calls and core compositions
to one commit per intrinsic. Include compound users when they are the remaining consumers of the
same legacy operation; isolate genuinely new ABI, resource, control-flow or synchronization behavior.
Use existing helpers and canonical IR. Delete recognizers/dispatch routes once all live consumers
have migrated. Do not retain duplicate signature tables or target-specific repairs for malformed IR.

## Fast validation by default

Choose the smallest checks that establish the batch's actual contracts:

- For a name migration through an unchanged qualified call path, run focused compiler units and
  compile representative scalar/aggregate/Half/Float32/Float64 inputs as applicable. Inspect NVVM
  LLVM/PTX output and compare relevant operations/call paths with NVRTC PTX. Whole PTX files need
  not be identical. This is a code-generation smoke check, not proof of numerical equivalence.
- Keep direct negative tests for removed IDs/tags/text and signature/no-mutation boundaries when
  affected. Reuse existing generic coverage instead of multiplying every check per operation.
- Run a small existing runtime selection when composition, casts, special-value behavior or the
  emitted operation changes. Require focused execution for synchronization, memory, ABI or control
  semantics that compile/PTX inspection cannot establish. Expand only in response to a concrete
  failure or unresolved risk.
- Reuse existing numerical fixtures and policies. Do not generate exhaustive independent oracles,
  baseline buffers, correction proofs or old-module campaigns for straightforward migrations that
  preserve the selected operation. New numerical algorithms need their own bounded correctness
  evidence; a name migration does not create a new algorithm.
- Build incrementally with eight jobs. Reuse unchanged binaries and successful test results tied
  to their actual identity. Do not rerun focused tests just because a broader suite includes them.
  Formatting/documentation/evidence updates do not require a compiler rebuild.

No full native, frozen/discovery, surface, toolkit or material campaign is required for each batch
or stopping point. Run broader checks at an integration milestone or when a shared change/failure
makes the affected scope uncertain. Name-list additions and operation-specific legacy-route removal
alone do not trigger them. Before publication, follow the repository's required CI/review checks.

Consolidate independent review around the batch diff, chosen checks and actual results. Routine
captures and record updates do not require separate approvals. Preserve failures and investigate
regressions; do not turn wrong output, crashes, missing execution or skips into passes. A PTX smoke
check must be recorded as such, never as fresh GPU or universal accuracy evidence.

## Module compatibility

Keep the already-tested log migration at semantic module version43, provider ABI46 and container
format2. **Do not bump the semantic module version for each further intrinsic migration.** The
prototype does not need a historical-module compatibility campaign for each source change; rebuild
stale modules against the current core, including earlier modules carrying the same version43.
This does not promise compatibility for every prototype binary with that version. Preserve numeric
holes, capability numbering and serialized layouts. Retired explicit operations may diagnose through
normal validation. Revisit versioning only for an actual serialized-format/decoding change or a separately
identified compatibility requirement. Do not rewrite old modules or silently accept invalid IR.

## Acceptance and current evidence

Record the actual commands, outcomes, tested source/binary identity and evidence scope concisely.
For targeted batches, retain the last full baseline unchanged and update current feature/status
records with focused evidence. Historical GPU/numerical/performance results keep their original
identities. A source/PTX smoke batch does not reset or claim a full checkpoint.

When a full checkpoint is deliberately selected, retain exact input/outcome comparisons and existing
failure histories. Use the frozen census and existing discovery/surface/material contracts; matching
totals alone are insufficient. Requalify changed environment components with the smallest relevant
checks before relying on them. The final log-family checkpoint already qualifies the upgraded host.

## Measurement

Use optimized, qualified builds, fixed options/inputs, warmups and repeated samples without competing
work. Separate fresh-process wall time, nested Slang phases, downstream assembly, shared-session
lifetime and GPU execution. Warm filesystem/toolkit/PCH caches are compatible with fresh processes;
record cache evidence and math options. Never sum inclusive nested timers or marginal medians.
Failed compilation latency is not successful compile time. PTX/cubin sizes, registers, stack/spills
and SASS are scoped observations, not GPU speed. Material runtime needs explicit binding/input/output
contracts. Preserve and restore accepted layouts around temporary profiling; no unpaired speed claims.

## Keep current information, not a second project history

Each retained artifact has one responsibility:

| Artifact                  | Update rule                                                                           |
| ------------------------- | ------------------------------------------------------------------------------------- |
| Architecture              | Current pipeline, ownership, invariants and rationale for surviving exceptions.       |
| Feature matrix            | Supported combinations, boundaries, evidence strength and permanent test links.       |
| STATUS                    | Authority, current identities/cadence, unresolved issues and next action; keep short. |
| RESULTS                   | Reusable commands and measurement/acceptance protocols.                               |
| Current accepted evidence | Exact outcomes, inventories, provenance and unresolved/resolved histories.            |
| Tests and input manifests | Reproducible contracts and selected inputs; frozen inventories remain immutable.      |

Update these artifacts in place. Plans, five-part report/PR-description drafts, per-attempt logs,
source snapshots, repeated samples, generated presentations and exhaustive audit indexes stay
uncommitted under ignored working paths or `build/`. Put an actual PR's five-part narrative in its
PR description. Use a concise commit body when closing local-only work. Do not append slice summaries
to the architecture or matrix, or create new permanent numbered plans/reports/evidence files.

Replace the current full baseline only after accepted full validation. Preserve all current cells,
input/runtime identities and unresolved/resolved failure history; Git retains previous accepted
snapshots. Keep focused evidence outside that baseline in one current feature-keyed record while it
supports a current claim or open issue. Replace superseded entries with reviewed evidence, retaining
failure transitions; remove redundant supporting files in the same change. Do not embed full old
baselines recursively. Unchanged evidence keeps its tested identity and never becomes fresh.

At closeout, ask of every added durable artifact: which current contract does it own, why is an
existing artifact insufficient, and when is it replaced? New durable categories need a concrete
consumer and a documented retention rule. Existing slice-named inventories are retained only where
live tools depend on their exact identity. Superseded material is recoverable through Git, not copied
into a checked-in archive. `.gitignore` guards against accidental numbered snapshot accumulation.

Format changed files, inspect the final diff, verify live links/inputs and exact evidence preservation,
and obtain independent review. Update current status and stop/resume authority, then make the
reviewed local commit. Do not commit completed working plans or report drafts. No Slack notifications.

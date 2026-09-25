# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 218 is accepted: FP32 masked min/max preserves its CUDA source algorithm.** Read the
[completed plan](plan.slice-218-minmax-algorithm.md), [five-part report](report.slice-218-minmax-algorithm.md)
and [result manifest](runtime-validation.slice-218.json). Scalar and aggregate leaves use caller-seeded
ordered comparison/selection, descending XOR butterfly stages for low contiguous power-of-two masks,
and ascending scans of original inputs otherwise. FP64 sum seed handling shares the mask classifier
but retains its behavior. Ordinary provider numeric min/max and FP64 admission remain unchanged.

All 288 cases from [research 217](semantic-evidence.slice-217.json) now match their unchanged independent
raw-word oracle, fixing its 96 mismatching executions. All 65,016 active output words and 64,008 inactive
sentinels match. The earlier [singleton finding](semantic-evidence.slice-215.json), fixed in 216, remains
covered. Both research findings are separate from the 53 registered failure histories.

**Next action: resume the deferred FP64 min/max semantic/admission gate.** Establish its independent
64-bit oracle and CUDA behavior before admitting scalar or aggregate helpers. Keep unrelated prefixes,
sum/product, vector-by-value shuffles and batching outside that slice. The discovery manifest is now
at its declared 100-identity maximum: before adding another registered source, make a separate bounded
capacity change with the required full checkpoint; do not bypass loader bounds or silently drop entries.
No push is authorized. Fresh-context delegation reached the app's agent-thread limit, so slices 217/218
used WORKFLOW's local fallback; no independent worker review is implied.

Latest accepted targeted implementation: 218. Latest full checkpoint: 214. Implementation slices since
that checkpoint: two (216 and 218). A third implementation requires a full checkpoint before a fourth;
broad/shared lowering, provider/library/ABI or runner contract changes trigger it sooner. Rolling
214/216/218 covers complex process lifetime and two proven correctness fixes; research 215/217 does
not advance cadence. Material runtime still needs its application bindings and output oracle.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                                 |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------- |
| Slice 218 accepted targeted | [Manifest](runtime-validation.slice-218.json), [frozen](census.slice-218.tsv), [discovery](discovery-census.slice-218.tsv) | 621 fresh cells: 583 correct, 38 retained failures. 1,035 frozen cells explicitly inherit 214. |
| Slice 216 accepted targeted | [Manifest](runtime-validation.slice-216.json)                                                                              | Singleton correction; all old fresh outcomes retained in 218.                                  |
| Slice 214 accepted full     | [Manifest](runtime-validation.slice-214.json)                                                                              | All 1,650 cells fresh: 1,597 correct and 53 known failures. Latest full checkpoint.            |

Frozen remains 452 identities/1,356 cells. Discovery now has 100 identities/300 cells. Cumulative
1,656 cells contain 1,603 correct and 53 unchanged failures; four resolved histories remain. All 618
old fresh cells match classification, return code, complete execution counts, diagnostic and canonical
shape exactly; three additions pass. No requested cell is missing, extra or duplicated. All 547 old
runtime source contracts are unchanged; one new fixture is added. Historical healthy denominators
427/72 stay fixed. All wave/quad and selected double/helper/value/vector/matrix neighbors run freshly;
other frozen results remain inherited 214 and are not represented as fresh passes on 218.

Fresh gates: focused 13/13, GPU smoke 4/4, units 478/478 plus one existing Windows-only skip,
toolkit 18/18, research replay 288/288 and six material compile/assembly cells. Frozen diagnostic mode
returns zero for eight retained preflight stops; discovery returns two for retained failures. Structured
outcomes decide acceptance. No material runtime correctness or performance claim is made.

## Unresolved failures and limitations

- All 53 open failure records and four resolved histories retain first-known evidence and reproduction.
- FP64 min/max admission, quad reconvergence, prefix-min/max
  KernelContext pointers and ordinary FP64 vector-by-value compound shuffles remain separate work.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- The material lacks application bindings, texture/LUT/input contract and runtime output oracle.
  Six cells compile/assemble; no material kernel correctness or speed claim.
- Slice 214 batching remains explicit opt-in. Mandatory fresh reference work made complete invocations
  slower despite 18.02355% paired compilation-lifecycle improvement; it is no routine accelerator.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 36. Use matching
optimized `build/RelWithDebInfo/{bin,lib}`, source `build/nvvm-loop/slice-203-env.sh`, and local
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Sequential suites, four corpus/build
workers maximum, two unit servers; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Tested base `5316e1b2516b9f9761a6386ed59296e5f758fa53` plus the final recipe/unit/fixture changes.
Compiler SHA256 `a13354a47acefc8684cbfb5ae52b84bcf0331a83e5310e9675f53a0e030107d7`.
Provider unchanged `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Raw evidence: `build/nvvm-loop/slice-218-before`, `slice-218-after`, including `research-replay`.
The new fixture's before/after SHA256 is `a4c91ebc19acbafcf8f6e21ef10eb314c5df7525de1f969e78cd0b52fac976e6`.
No GPU loss, driver change, reboot or push. Local parent review/acceptance follows the recorded
fresh-context delegation limitation.

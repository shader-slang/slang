# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 227 is accepted: source-order FP64 min/max prefixes execute.** Read the
[completed plan](plan.slice-227-fp64-prefix.md), [five-part report](report.slice-227-fp64-prefix.md)
and [targeted result manifest](runtime-validation.slice-227.json). Canonical scalar/vector FP64
inclusive/exclusive min/max now preserve ordered operand selection, exact infinity/caller seeds,
and the source shuffle-up/ascending-scan distinction. Separate transmitted inclusive state preserves
NaN payloads and signed zeros. Existing reduction/arithmetic recipes and FP32 prefixes are unchanged;
provider ABI 36 is unchanged.

**Next action: research narrow-integer masked min/max prefixes in slice 228.** Original frozen min/max tests now reject `int8_t(int8_t, vector<uint,4>)` canonical
`_wavePrefixExclusiveMin/Max(($1).x, $0)` instead of double. Retain this independent blocker and
establish its source identities, promoted arithmetic/selection and exact semantics before extending
admission. Matrix prefix capability, ordinary FP64 vector-by-value shuffles, quad reconvergence and
resource-bearing contexts remain separate. Reconsider material-driven work; material runtime still
requires application bindings, texture/LUT/input and an output oracle. No push is authorized.

Latest full checkpoint: 225. Latest targeted implementation: 227.
Implementation slices since full: one. Rolling feature history: 223 CUDA bit-index
correctness, 225 copyable contexts, 227 FP64 source min/max prefixes. Correctness and research-backed
runtime support take priority while material execution lacks its application contract. Discovery
capacity remains 50–128, with 104 current identities. One fresh-context worker implemented 227;
the parent independently reviewed its diff and evidence before the authorized local commit.

## Checkpoints and evidence

- [Slice 227 targeted](runtime-validation.slice-227.json): 633 fresh cells, 1035 explicitly inherited
  from 225; cumulative 1668 cells, 1617 correct and 51 known failures.
  [Frozen census](census.slice-227.tsv); [discovery census](discovery-census.slice-227.tsv).
- [Research 226](semantic-evidence.slice-226.json): exact source prefix semantics and eight before
  direct rejections. Slice 227 replays all 196 unchanged inputs/expectations in each of three modes:
  588 exact executions, 172,872 active binary64 results and 353,976 inactive sentinels. Input buffers
  stay unchanged; three PTX artifacts assemble.
- [Slice 225 full](runtime-validation.slice-225.json): 1665 fresh cells, 1614 correct, 51 known failures.
- [Research 224](semantic-evidence.slice-224.json): context findings resolved by 225.
- [Slice 223 full](runtime-validation.slice-223.json), [slice 221 targeted](runtime-validation.slice-221.json).

Frozen remains 452 identities/1356 cumulative cells: 1335 correct, 16 preflight and 5 infrastructure
failures. Its selected 107 identities/321 cells are fresh; 1035 cells inherit full 225. Discovery has
104 identities/312 fresh cells: 282 correct, 22 infrastructure, 4 mismatch and 4 preflight failures.
All 1614 previous correct cells are preserved, fresh or explicitly inherited; the new fixture adds
three correct cells. Exactly four old prefix diagnostics advance from double to int8_t; all other
fields remain unchanged. All 551 previous runtime input hashes and all 103 old discovery rows remain
unchanged. No missing, extra or duplicate cells. Historical healthy denominators 427/72 remain fixed.
All 51 open first-known failure records and six resolved histories are retained, including prior
prefix diagnostic observations.

Fresh gates: focused 3/3, GPU smoke 4/4, units 479/479 plus one existing Windows-only skip, toolkit
18/18, discovery contracts 6/6, research 588/588 and material compile/assembly 6/6. The final fixture
passes source mode on the rebuilt original emitter and rejects both direct modes before the change.
The initial fixture-only implicit-conversion warning and the clean revert drill are retained. Frozen
selection returns 0 in diagnostic mode; discovery returns 2 for known failures. Structured outcomes
decide acceptance. No material runtime or performance claim is made.

## Unresolved failures and limitations

- Four original prefix cells now expose narrow-integer exclusive min/max GenericAsm; no int8 fix.
- Matrix prefix overloads require glsl_spirv and reject CUDA before emission, as recorded in 226.
- Quad reconvergence, ordinary FP64 vector-by-value shuffles and resource-bearing contexts remain.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- The material lacks its application runtime contract; its six cells compile/assemble only.
- Slice 214 batching remains opt-in, not a routine accelerator; no new performance claim.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 36. Use matching
optimized `build/RelWithDebInfo/{bin,lib}`, source `build/nvvm-loop/slice-203-env.sh`, and local
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Sequential suites, four workers maximum,
two unit servers; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Tested base `29a47957226cf56767bd0f509a1ce56f65f7cea1` plus recorded emitter/fixture/discovery changes.
The result manifest records all 27 source, 12 artifact and 552 runtime-input hashes. The original
emitter was rebuilt for the clean before proof; exact source and binary identities are retained.
Provider hash remains `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Raw evidence: `build/nvvm-loop/slice-227-before` and `slice-227-after`; research226 raw artifacts are
unchanged. No GPU loss, driver change, reboot or push. Parent acceptance and local commit remain.

Parent acceptance verified 216 evidence references, 27 source hashes, 12 artifact hashes and
552 runtime input hashes, including unchanged prior inputs and complete first-known failure history.

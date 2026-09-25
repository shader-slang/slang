# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Research 226 is accepted: FP64 prefix source semantics are established.** Read the
[report](report.slice-226-fp64-prefix.md), [plan](plan.slice-226-fp64-prefix.md) and
[evidence](semantic-evidence.slice-226.json). All 196 source cases match 57,624 active binary64
results and 117,992 inactive sentinels. Eight direct probes reject before PTX. A plain ascending
scan differs in 8,658 words; preserve the source shuffle-up tree and separate transmitted state.
Matrix prefix capability rejection is a separate recorded limitation.

**Slice 225 is accepted: canonical thread-local contexts admit supported copyable values.** Read the
[completed plan](plan.slice-225-copyable-context.md), [five-part report](report.slice-225-copyable-context.md)
and [full result manifest](runtime-validation.slice-225.json). The existing recursive copyable-struct
classifier replaces the older scalar-only context restriction. Exact pointer qualifiers remain;
provider ABI 36, storage lifetime, helper pointer matching and resource-bearing context restrictions
are unchanged. Dynamic Boolean/nested/double/vector/array state passes all three runtime modes.

**Next action: implement source-order FP64 inclusive/exclusive min/max prefixes in slice 227.**
Use caller seeds for inclusive prefixes and binary64 ±infinity for exclusive prefixes, ordered
compare/select, shuffle-up transmitted/returned state for low contiguous power-of-two masks and
ascending original-input scans otherwise. Reuse existing mask classification and typed operations;
preserve reduction/arithmetic recipes. Prove before/after output and replay the exact research226
inputs/oracle. Reassess original frozen min/max workloads; stop at any independent next blocker.
Matrix capability and resource-bearing contexts stay separate. No direct prefix runtime support
is claimed yet. Select a justified affected regression domain; full225/cadence0 remain current.

No push is authorized. Fresh-context delegation remains at the app's agent-thread limit; local work
uses WORKFLOW's fallback and does not imply independent worker review. Material runtime still needs
application bindings, texture/LUT/input and output oracle; independent backend work can continue.

Latest full checkpoint and implementation: 225. Latest targeted implementation: 221.
Implementation slices since full: zero. Rolling feature history: 221 FP64 source min/max reductions,
223 CUDA bit-index correctness,225 executable copyable contexts. Correctness and research-backed
runtime support take priority while material execution lacks its application contract. Reconsider
material-driven work at each selection. Discovery capacity remains 50–128, with 103 current identities.

## Checkpoints and evidence

- [Slice 225 full](runtime-validation.slice-225.json): 1,665 fresh cells, 1,614 correct, 51 known failures.
  [Frozen census](census.slice-225.tsv); [discovery census](discovery-census.slice-225.tsv).
- [Research 224](semantic-evidence.slice-224.json): canonical Boolean context restriction isolated;
  exact replay in 225 now passes all nine executions and 864 checked words.
- [Slice 223 full](runtime-validation.slice-223.json): previous 1,662-cell preservation checkpoint.
- [Slice 221 targeted](runtime-validation.slice-221.json): FP64 source min/max admission.

Frozen remains 452 identities/1,356 cells: 1,335 correct, 16 preflight and 5 infrastructure failures.
Discovery has 103 identities/309 cells: 279 correct, 22 infrastructure, 4 mismatch and 4 preflight failures.
All 1,611 previously correct cells remain correct; three additions pass. Exactly four old prefix cells
change only diagnostic/canonical shape; all other 1,658 old cells preserve all five outcome fields.
All 550 previous runtime input hashes and 102 old discovery rows are unchanged. No missing, extra,
duplicate or inherited runtime cells. Historical healthy denominators 427/72 stay fixed. All 51 open
first-known failure records and six resolved histories remain intact, including prior diagnostic
observations for the four advancing prefix cells.

Fresh final gates: focused 6/6, GPU smoke 4/4, units 479/479 plus one existing Windows-only skip,
toolkit 18/18, discovery contracts 6/6, research 9/9 and material compile/assembly 6/6. Three initial
fake-provider failures exposed missing Boolean leaf/load/unary bookkeeping; retained logs document
the repairs, and final gates use the rebuilt test artifact. Both corpus runners return 2 for known
failures; structured outcomes decide acceptance. No material runtime or performance claim is made.

## Unresolved failures and limitations

- Matrix prefix overloads require glsl_spirv and reject CUDA before emission; research226 retains
  the exact diagnostic. Scalar/vector prefix experiments do not establish matrix support.

- All 51 open failure records and six resolved histories retain first-known evidence and reproduction.
- FP64 masked prefix min/max, quad reconvergence and ordinary FP64 vector-by-value compound shuffles
  remain separate work. Resource-bearing explicit contexts remain outside the copyable-only branch.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- The material lacks its application input/output contract; six cells compile/assemble only.
- Slice 214 batching remains explicit opt-in. Mandatory fresh reference work made complete invocations
  slower despite 18.02355% paired compilation-lifecycle improvement; it is no routine accelerator.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 36. Use matching
optimized `build/RelWithDebInfo/{bin,lib}`, source `build/nvvm-loop/slice-203-env.sh`, and local
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Sequential suites, four corpus/build
workers maximum, two unit servers; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Tested base `5cd4c301896960cf4b834e67dfaf7ad92194d41e` plus recorded context/unit/fixture changes.
Compiler SHA256 `14e80c03ff1571a2248f935cc795a9465ec963f9d3ac5cf4a7f80ba076a48ae1`.
Provider unchanged `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Raw evidence: `build/nvvm-loop/slice-225-before` and `slice-225-after`, including exact research224
replay and original prefix probes. Parent verified 203 evidence references, 26 tested source hashes,
12 artifact hashes and 551 runtime input hashes before recording acceptance. No GPU loss, driver
change, reboot or push. Local parent review follows the recorded fresh-context delegation limitation.

Research226 raw evidence: `build/nvvm-loop/slice-226-prefix`. GPU smoke4 and source probe196
executions are fresh; registered outcomes and other gates inherit unchanged full225. All26 source,
12 artifact and551 input hashes were rechecked. No binary rebuild or provider/ABI change.

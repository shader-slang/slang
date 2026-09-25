# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 230 research is accepted.** Read the
[completed plan](plan.slice-230-i64-minmax.md), [five-part report](report.slice-230-i64-minmax.md)
and [semantic evidence](semantic-evidence.slice-230.json). Signed/unsigned 64-bit masked MIN/MAX
reductions and inclusive/exclusive prefixes match independent integer member-set extrema in CUDA
source mode, including scalar/vector2/vector4 and existing matrix2x2 reduction leaves. Typed 64-bit
lane transport, MIN/MAX, SELECT and constants pass source and direct O0/O3 controls. No compiler,
provider or corpus change was made; ABI 36 remains unchanged.

**Next action: implement the bounded 64-bit integer MIN/MAX recipe extension.**
The 80 minimal direct family probes retain canonical E52017/noPTX. The responsible boundary is
`_getNVVMMaskedWaveScalarIdentity`; safe width 64 identity materialization is also required. Avoid
shift-by-64 in unsigned maximum and signed argument conversion. Reuse existing scalar/aggregate
recipes, typed provider operations and bit-preserving conversion. Matrix-prefix capability,
arithmetic/bitwise/FP16, ordinary aggregate shuffle policy, quad reconvergence and resource contexts
remain separate. This worker stops at the research gate. Material runtime still needs its application
bindings, texture/LUT/input and output oracle. No push is authorized.

Slice 229 remains the latest accepted implementation: [plan](plan.slice-229-narrow-minmax.md),
[report](report.slice-229-narrow-minmax.md), [full validation](runtime-validation.slice-229.json).

Latest full checkpoint: 229. Latest implementation: 229.
Implementation slices since full: zero. The shared aggregate classifier change
triggered a full checkpoint rather than the initially planned targeted run. Rolling feature history:
225 copyable contexts, 227 FP64 source min/max prefixes, 229 narrow integer masked min/max.
Correctness and research-backed runtime support take priority while material runtime lacks its
application contract. Discovery capacity remains 50–128, with 105 current identities.

## Checkpoints and evidence

- [Research 230](semantic-evidence.slice-230.json): 2,128 source MIN/MAX and 6,384 typed-control launches,
  all 8,512 exact; 14,981,120 output words including inactive sentinels, inputs unchanged. Eight PTX
  assemblies and eight catalog contracts pass; 80 direct E52017 and 24 matrix capability rejections
  retain no PTX. Fresh smoke 4/4. All 28 source/12 artifact/553 registered input hashes unchanged.
  All slice 229 registered/gate/failure evidence below is inherited, not refreshed; cadence remains zero.

- [Slice 229 full](runtime-validation.slice-229.json): 1,671 fresh cells, 1,620 correct and 51 known
  failures. [Frozen census](census.slice-229.tsv); [discovery census](discovery-census.slice-229.tsv).
  No inherited runtime cells. All 1,617 previous correct cells are preserved; one fixture adds three.
- [Research 228](semantic-evidence.slice-228.json): independent narrow prefix source semantics,
  17,472 exact launches and 64 before direct rejections. Slice 229 replays every old input and
  expectation, including all 32-bit controls, in the expanded three-mode inventory.
- [Slice 227 targeted](runtime-validation.slice-227.json): 633 fresh and 1,035 inherited cells;
  cumulative 1,668 cells, 1,617 correct and 51 known failures. This overlay on
  [full 225](runtime-validation.slice-225.json) remains the preservation source for 229.

Frozen remains 452 identities/1,356 cells: 1,335 correct, 16 preflight and five infrastructure
failures. Discovery has 105 identities/315 cells: 285 correct, 22 infrastructure, four mismatch and
four preflight failures. Exactly four old prefix diagnostics advance; all other old classification,
return-code, execution-count, diagnostic and canonical-shape fields are unchanged. All 552 old
runtime input hashes and all 104 old discovery rows are unchanged. No missing, extra or duplicate
cells. Historical healthy denominators 427/72 remain fixed. All 51 open first-known failure records
and six resolved histories are retained, with the full prefix diagnostic history appended.

Slice 229 fresh gates (inherited by research 230): focused 3/3, GPU smoke 4/4, units 479/479 plus one existing Windows-only skip,
toolkit 18/18, discovery contracts 6/6 and material compile/assembly 6/6. The complete final emitter
patch was reversed and rebuilt: the final unchanged fixture passes NVRTC and rejects direct O0/O3.
Intermediate narrow aggregate E52017 and identity-construction E52018 evidence explains both
cascading fixes. Eighty excluded operation/width signatures and eight narrow aggregate shuffle
signatures preserve their before/after rejections.

Slice 229 separate research replay (inherited): 47,712 primary prefix launches, 672 wide-input prefix launches and 4,704
reduction launches all match independent expectations, totaling 53,088 launches and 46,663,680
output words. Inputs and inactive sentinels are checked. All 18 prefix and 12 reduction PTX artifacts
assemble; 64 minimal prefix and 32 minimal reduction direct compilations now succeed. Reduction
before proof covers 1,568 source launches, including matrix leaves and truncation, with 32 direct
rejections. Research launches are separate from registered runtime cells.

Full frozen and discovery both return 2 for retained failures. Structured results
determine acceptance. No material runtime or performance claim is made.

## Unresolved failures and limitations

- Four original prefix cells expose `int64_t` exclusive min/max GenericAsm (E52017); research 230 establishes the source contract without changing their outcomes.
- Matrix prefix overloads still reject CUDA capability before emission; matrix reductions are covered.
- Quad reconvergence, ordinary FP64 vector-by-value shuffles and resource-bearing contexts remain.
- Hardware masks remain scheduling-dependent; logical active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible.
- The material lacks its application runtime contract; its six cells compile/assemble only.
- Slice 214 batching remains opt-in; no new performance claim.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 36.
Use matching optimized `build/RelWithDebInfo/{bin,lib}`, source
`build/nvvm-loop/slice-203-env.sh`, and local `build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`.
Sequential suites, at most four workers, two unit servers; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Research 230 tested accepted source `ef0cd6bf92bced6c52245337f3abc500644ae6a9` with no compiler changes.
Slice 229 historically tested base `16207a0c3781d8e9158f205aee2e48e68cc11c3a` plus its recorded changes.
Final compiler-library SHA-256:
`577c8eea1039a9b6090db4bfd993bc143f53bb78d5c75f9d4d26b4753e8a11af`.
Provider remains `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
The result manifest retains 28 source, 12 artifact and 553 current runtime-input hashes.
Raw evidence: `build/nvvm-loop/slice-229-before` and `slice-229-after`; older evidence remains intact.
No GPU loss, driver change, reboot or push. Parent acceptance verified 192 unique evidence
references, all source/artifact/input hashes, all 1,671 fresh outcomes, complete failure histories
and all 53,088 semantic replay launches. Slice 229 is committed as the tested research base above.

Research 230 raw evidence: `build/nvvm-loop/slice-230-i64-minmax`. Parent acceptance verified 225 unique
evidence references and independently reconstructed all 8,512 input/expectation hashes. All recorded
source, artifact and input hashes remain unchanged. No implementation slice started during research.

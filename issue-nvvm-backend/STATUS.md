# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 235 is accepted as the latest full checkpoint.** The full validation and independent parent
audit pass. Read the [completed plan](plan.slice-235-quad-helpers.md),
[five-part report](report.slice-235-quad-helpers.md) and
[full validation](runtime-validation.slice-235.json). Typed CUDA `_slang_quadAny`/`_slang_quadAll`
helpers now emit the source prelude's four full-mask indexed reads through existing operations.
Canonical target lookup and exact bool(bool)/whole-body validation own both requirement markers;
standalone markers, mismatched helpers/signatures and unrelated body instructions still reject.
No provider/ABI, frontend, standard-library or runner change.

The two original frozen quad-control direct cells are now GPU-correct. The new dynamic fixture
passes all three modes. Latest accepted implementation and full checkpoint: 235. Latest targeted
acceptance: 233. Implementation slices since full: zero. Rolling feature history is 231 I64 MIN/MAX,
233 FP16 MIN/MAX and 235 quad helpers.

**Research 236 is accepted.** [Clock report](report.slice-236-clock.md),
[plan](plan.slice-236-clock.md) and [semantic evidence](semantic-evidence.slice-236.json) establish
exact `clock` uint() / `clock64` int64_t() contracts. Naive LLVM intrinsics common at O0/O3 and
hoist at O3 through libNVVM12.9. All 36 source/side-effecting PTX control launches pass; 18 intrinsic
counterexamples remain explicit research failures. Three reconstruction modes and 20 assemblies
pass. All 31 source, 12 artifact and 556 input hashes preserve full235; the three original frozen
clock cells preserve exact outcomes. No implementation, ledger or cadence change. Delegation was
unavailable; the recorded local fallback uses a separate raw-buffer acceptance checker.

Next action: implement narrowly typed side-effecting clock provider operations, strict admission
and negative tests, and an independently expected runtime fixture. Preserve/replay research236
inputs and relational predicates without comparing nondeterministic ticks. A provider contract
change forces a full frozen/discovery/material support checkpoint. Material runtime bindings,
textures/LUT/input and output oracle remain absent; no runtime/performance claim or push.

## Checkpoints and evidence

- [Full 229](runtime-validation.slice-229.json), [targeted 231](runtime-validation.slice-231.json)
  and [targeted 233](runtime-validation.slice-233.json) remain immutable comparison inputs.
  Prior cumulative acceptance was 1,677 cells, 1,630 correct, 47 unresolved and ten resolved histories.
- Full 235 reruns every frozen identity from the immutable slice-195 inventory: 452 identities /
  1,356 cells, with 1,341 correct, five infrastructure and ten preflight outcomes. Only the original
  quad-control O0/O3 cells change, from E52017 preflight to GPU-correct. Every other old five-field
  outcome is exact. [Frozen census](census.slice-235.tsv).
- Discovery reruns all 107 old identities and adds one: 108 identities / 324 cells, with 294 correct,
  22 infrastructure, four mismatch and four preflight outcomes. All 321 old outcomes preserve
  classification, return code, complete execution counts, diagnostic and canonical shape; the new
  fixture contributes three separate correct cells. [Discovery census](discovery-census.slice-235.tsv).
- Total 1,680 fresh cells, 1,635 correct, 45 retained failures and twelve resolved histories.
  No missing, duplicate, extra or inherited cells, lost support, oracle change or baseline reset.
  All ten older resolved histories remain intact; each of the two new histories retains its exact
  whole prior failure record, transition and fresh proof. First-known records and reproductions
  for the 45 remaining failures remain intact. Historical healthy denominators 427/72 are fixed.
- Fresh final-source smoke 4/4 precedes expensive suites; focused 3/3, units 479/479 plus one existing
  Windows-only skip, toolkit 18/18, runner contracts 6/6 and all six material compile/assembly cells
  pass. Eight negative cases extend the existing unit matrix and prove rejection before provider
  loading/module creation. Six canonical alias compiles pass; sixteen direct negative probes and
  four standalone CUDA-source E99999 cases retain exact diagnostics.
- The final readable dynamic fixture passed NVRTC and rejected both direct modes on the accepted
  compiler before production edits. Its source and TEST_INPUT directives never changed afterward.
  It checks all Boolean truth tables, divergent inline/noinline calls and complete-quad exits using
  independent table-derived expectations.
- Exact [research 234](semantic-evidence.slice-234.json) replay reads 512 unchanged binary input /
  expectation pairs. All 3,072 fresh launches pass 196,608 output words, with unchanged 96-word input
  regions and inactive sentinels. Each public/control × NVRTC/O0/O3 group has 512 cases. All 2,048
  prior actual-buffer hashes are preserved, with 1,024 additional direct public-helper launches.
  Twelve replay PTX artifacts assemble. Accepted research raw files remain untouched.

## Unresolved failures and limitations

- CUDA requires complete source quads and matching shuffle sequences by named non-exited lanes.
  SM80 divergent rendezvous is tested; missing source lanes or unmatched sequences have no oracle.
  There is no CUDA-wide maximal reconvergence, active-only vote or SM6x divergent-behavior claim.
- Matrix prefix capability, other arithmetic/bitwise families, ordinary aggregate shuffle policies
  and resource-bearing contexts remain independent. Hardware active-mask synthesis is unchanged.
- FP8/BF16/prelude and existing discovery infrastructure/output gaps remain visible in the ledger.
- Material's six cells are freshly checked compile/assembly support only. Runtime/performance claims
  remain blocked by the missing application contract. Slice 214 batching remains opt-in.

## Environment and final evidence

Native Ubuntu 24.04, branch `nvvm-backend`, repository `/home/skallweit/codex/agent-sandbox/slang`.
L4 SM89, driver 580.126.09, target SM80, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, provider ABI 36.
Use matching optimized `build/RelWithDebInfo/{bin,lib}`, source the inspected
`build/nvvm-loop/slice-203-env.sh`, and follow the local
`build/nvvm-setup/slang-skills/skills/slang-build/SKILL.md`. Maximum four CPU workers, two unit
servers, sequential suites and 30-minute bounds; `CMAKE_BUILD_PARALLEL_LEVEL=1`.

Both source_commit and source_revision are actual base
`2cf7d42e0c259c05bc0fd7ab38e9490d05f0155e` plus the recorded patch. Final compiler-library SHA-256:
`ca34db1a349ae8716785032a0a3b01b3e6cf8455f3137e9358e9d1ad4eca63cf`.
Before compiler: `92ae81d069aeda9a6ff2a61edec43f572b2af02bb7ec677fc444490ea9a966f1`.
Provider remains `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Every final gate captures all 30 old source paths, 12 artifacts and the new fixture hash; only
emitter, negative unit source and discovery registration change among old sources. All 555 old
registered input hashes remain exact; the fixture adds input 556 and source path 31.

Raw evidence is under `build/nvvm-loop/slice-235-before` and `slice-235-after`; `audit.json` records
full inventory/history preservation, immutable research evidence and independent raw-buffer checks.
The first recipe attempt used the wrong lane signedness in its descriptor; its exact source patch,
identity and failed focused result remain under `attempt-1`. All gates reran on the corrected source.
An evidence-script edit later interrupted the gate shell after passing units; a separate recorded
resume script ran only the still-unrun gates on identical source/artifacts. No incomplete command
was counted as a pass. No GPU loss, system change, reboot, worker commit or push.

Parent acceptance verified 689 unique compact evidence references, all 31 current sources, 12
artifacts and 556 registered inputs, preserving all 555 old input hashes. Exact full census review
confirmed only two resolved old cells and three additions. All 3,072 raw replay buffers match
accepted research expectations and preserve every old actual-output hash. Full checkpoint 235
resets implementation cadence to zero. See `parent-acceptance-audit.json` under the final raw run.

# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before resuming the loop.

## Current state and next action

**Implementation244 and its full checkpoint are accepted after independent parent review.** Read
[plan244](plan.slice-244-fp8-finite-producers.md), [report244](report.slice-244-fp8-finite-producers.md),
[validation244](runtime-validation.slice-244.json) and the [FP8 contract](../docs/design/nvvm-fp8-scalar-contract.md).

Four shared math helpers now correctly narrow and widen finite FP8 values, including subnormals and
E4M3 values256..448. Existing E4 overflow-to-NaN and E5 rounded-overflow-to-infinity policies remain.
The repair is at the canonical literal producer; no backend admission or provider ABI change.

Latest accepted implementation/full checkpoint:244. Latest targeted acceptance:233. Implementation
slices since full:zero. Tested source base `41f070c689cb46e91b3939ca038654b91ef96944`. Parent has
accepted the slice for the authorized local commit; no push is authorized. Rolling implementation
history:241vectors/242dot/244FP8producerfix.

After the local commit, re-rank the measured remaining boundaries. Material runtime still lacks
bindings, textures/LUTs, inputs and an output oracle. This producer-correctness slice overrides the
complex-corpus cadence; reconsider measurable material compile-time work next, which does not need
an application runtime contract. Narrow FP8 scalar admission, texture GetDimensions, arbitrary
RequirePrelude and dynamic-object aggregates remain independent candidates. BF16 integer construction
has a recorded double-rounding counterexample. BF vector storage needs the separate CUDA layout
producer repair and BF3 representation decision described by the BF16 vector contract.

## Checkpoint and proof

- Full244:452 frozen identities/1356 cells and113 discovery identities/339 cells. Total1695 fresh
  cells,1654 correct,41 unresolved and16 retained resolved histories. All1692 old five-field outcomes
  match accepted242 exactly; all1651 old correct cells survive. One new fixture adds3 correct cells.
- The52-word literal fixture had32 incorrect words in each mode before repair. All156 final GPU
  output words pass. Final linked IR contains ordinary32-bit outputs; it needs no FP8 admission.
- Independent shared-helper proof checks3548 cases, changing220 mismatches to zero. Two actual Slang
  NVRTC dynamic controls preserve24290 complete words:6072 active and18218 inactive words. Their
  CUDA SATFINITE oracle is distinct from the preserved shared-helper overflow policy.
- Smoke4, focused3, units511 plus one existing Windows skip, toolkit18 and runner contracts6 pass.
  All6 material compile/assembly cells pass; these are support checks, not runtime or speed claims.
- Parent separately verifies716 compact /1275 total evidence references before its seven own
  references, all58 current indexed artifacts, exact corpus and failure-history preservation, and
  all19 historical primary-source snapshots. Earlier indices238/240/241/242/243 remain immutable.

Unresolved failures and their original reproduction/history remain in validation244. Historical
BF16 vector/dot/export and research243 raw LLVM controls retain their original source/artifact
identities; they are not claimed as fresh244 controls. Registered corpus results are entirely fresh.

## Research and remaining semantic boundaries

[Research243](semantic-evidence.slice-243.json) qualified format-distinct i8 scalar/internal-helper
transport and CUDA12.9 RNE/SATFINITE Float32 casts on SM80:14 launches and170030 complete words.
That historical qualification remains intact. Its finite producer defects are repaired by244.
Shared literal overflow policy still differs from CUDA runtime SATFINITE; resolving that policy is
separate from finite correctness and from backend scalar admission. FP8 vectors, storage, aggregates,
dynamic objects and external ABI remain separate. ABI40 BF16 source-ordered dot remains accepted.

## Environment and evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86,
LLVM14, providerABI40 and matching RelWithDebInfo. Use inspected slice-203-env.sh and local build
skill. Four CPU workers maximum total, sequential GPU suites and30-minute bounds.

Compiler executable SHA256 `b9e87811c263f131cf1372b307bd60acdcc51993aaab2af91a0cfd03462d10a1`.
Provider SHA256 `c0522674424c86dbc9444b2abc202c97146a6b41e3a9179d95d34ec9fe1b0773`.
All43 source paths,12 artifacts and561 runtime inputs match before every final gate and afterward;
all560 old input hashes are exact. Seven artifacts changed through the shared-header rebuild;
the provider binary remains unchanged.

Raw roots `build/nvvm-loop/slice-244-before` and `slice-244-after` retain complete evidence.
See `parent-acceptance-audit.json`, `parent-helper-audit.json`, `parent-runtime-audit.json`, their
scripts, `fixture-output/results.json`, `trace/shape-proof.json`, `artifact-index.json` and compact
validation244. Live historical math source hashes intentionally change; snapshot hashes do not.
No GPU loss, driver/system change, reboot or push occurred.

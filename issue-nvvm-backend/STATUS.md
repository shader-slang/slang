# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before resuming the loop.

## Current state and next action

**Implementation241 and its full checkpoint are accepted after independent parent review.** Read [plan241](plan.slice-241-bf16-vector-values.md), [report241](report.slice-241-bf16-vector-values.md), [validation241](runtime-validation.slice-241.json) and [the vector contract](../docs/design/nvvm-bf16-vector-contract.md).

The slice adds canonical BF16 widths2/3/4 as register values and internal by-value helper arguments/results, construction/splats/extraction, helper branch/phi and matching-lane Float32 conversion. BuilderABI39 preserves BF16 identity and uses i16 vector values. Existing source bitcast lowering handles uint32/BF2, uint64/BF4 and ushort3/BF3. Recursive numeric/copyable/helper/storage classifiers remain unchanged. Explicit exported CUDA BF-vector helper parameters/results reject at preflight; internal signatures are not external CUDA ABI promises.

The frozen scalar-bf16 workload still stops at `GenericAsm assembly=_slang_vector_dot, signature=BFloat16(vector<BFloat16,4>, vector<BFloat16,4>)` in both direct modes. No old failure resolves. Source-ordered dot is the next concrete BF16 boundary; consult accepted [research238](semantic-evidence.slice-238.json). Exact integer construction, vector Select, arithmetic/comparison and Half/double casts remain separate. New vector local pointers/storage, aggregates/resources/globals/parameter groups remain excluded. Before storage work, repair the BF4 CUDA AST/IR layout producers' alignment8 versus actual prelude2; BF3 CUDA6/2 also differs from LLVM allocation8/alignment8.

Latest accepted implementation/full checkpoint:241. Latest targeted acceptance:233. Implementation slices since full:zero. Tested source base `3e218a92a6c3b3c2cdc02fdf854f03b9712e6dda`; no push is authorized. The slice is ready for the authorized local commit.

Rolling implementation history:237 clock observations,239 scalar BF16,241 BF16 vector values. Material-driven runtime work remains deferred because bindings, textures/LUTs, inputs and an output oracle are absent; reconsider that contract before each next slice.

## Checkpoint and proof

- Full241 is entirely fresh:452 frozen identities/1356cells and110 old discovery identities/330cells plus one new fixture/3cells. Total1689cells/1646correct/43unresolved;14resolved histories preserved. All1643 old correct cells survive. The only two deltas are the BF16 direct diagnostics/canonical shapes advancing from helper parameter to `_slang_vector_dot` GenericAsm. Full classification, return_code and execution_counts stay exact. Entire prior failure records remain in [validation241](runtime-validation.slice-241.json).
- Final smoke4, fixture3, existing exported-integer helpers3, units483 plus one old Windows skip, toolkit18, runnercontracts6 and all6 material compile/assembly cells pass. Material runtime/performance remains blocked by absent bindings/textures/LUT/inputs/oracle; support/correctness work is the recorded cadence override.
- Nine public value-only projections and six separate original raw controls pass all52,696,815 words. Each width carries73190 records: everyBF16 encoding plus7654 Float32 boundaries in every lane. Independent worker/parent integer oracles agree. NarrowingNaN is classification-only; SM80 widening preserves exact payload bits. Removed local-copy output columns retain original sentinels. Accepted240/238 artifacts remain unchanged.
- Final linked IR retains mixed constructors, multi-lane swizzle and dynamic extraction; all three exhaustive helpers retain BF-vector merge parameters. Provider unit serialization covers both dialects. Export preflight negatives and neighboring ordinary export checks pass. Earlier provisional exported-ABI PTX has no captured provisional compiler hash due a capture-script error and is investigation evidence only; final gate identities are complete.

## Environment and evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86, LLVM14, providerABI39 and matching RelWithDebInfo. Use inspected slice-203-env.sh and local build skill. Four CPU workers maximum total, sequential GPU suites and30-minute bounds.

Compiler SHA256 `79f46ef0dba116bdfdce8a03b40f7d3bb2c7b481529b452b483d650cf0958e43`. Provider SHA256 `116df24297dddc55b9c8f2f4f45f30e614e3185dafc618b3b67caf6df14a2bfe`. All39 tested sources,12 artifacts and559 inputs match before every final gate and after; all558 old input hashes are preserved.

Raw roots `build/nvvm-loop/slice-241-before` and `slice-241-after` retain baseline proof, commands/source/IR/PTX/buffers, complete hashes and full-checkpoint logs. See final `audit.json`, `replay-audit.json`, `frozen-comparison.json`, `trace/shape-proof.json`, `value-replay/results.json`, `artifact-index.json` and compact validation. Attempts and accepted research remain intact. No frontend/library/runner edits, GPU loss, system change, reboot, worker commit or push occurred. Independent parent acceptance verifies703 compact and1,303 total evidence references before adding its six own references, all88 current indexed artifacts,117 research238 and132 research240 artifacts, exact corpus/history preservation and52,696,815 replay words. See `parent-acceptance-audit.json`, `parent-oracle-audit.json` and `parent-raw-oracle-audit.json`.

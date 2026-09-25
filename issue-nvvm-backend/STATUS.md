# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before resuming the loop.

## Current state and next action

**Implementation242 and its full checkpoint are accepted after independent parent review.** Read
[plan242](plan.slice-242-bf16-dot.md), [report242](report.slice-242-bf16-dot.md),
[validation242](runtime-validation.slice-242.json) and [the BF16 vector/dot contract](../docs/design/nvvm-bf16-vector-contract.md).

ABI40 adds only source-ordered BF16 dot with equal vector operands widths2/3/4 and scalarBF16 result.
Canonical helper mapping reuses the existing operation planner; the provider emits separately rounded
SM80 BF16 products/sums in lane order from positive zero. The original frozen scalar-bf16 workload
now passes NVRTC and directO0/O3, resolving both complete old histories. No later blocker is exposed
in that source. No frontend/library, type-role, generic arithmetic, storage or external ABI admission
was added. Scalar239/vector241 behavior and exported BF-vector rejections remain preserved.

Latest accepted implementation/full checkpoint:242. Latest targeted acceptance:233. Implementation
slices since full:zero. Tested source base `d6c26eb4cf5960ac07feff8158d15163cc2757fc`; no push is
authorized. Implementation242 is locally committed at `bb99ea8c9a414fc4ee854422cbe8827be018f7c3`. Rolling implementation history:
239scalar/241vectors/242dot.

Re-rank remaining measured corpus boundaries before the next slice. FP8-containing helper resultA,
texture GetDimensions and arbitrary RequirePrelude remain independent; no new investigation was
undertaken here. Exact BF16 integer construction has the accepted double-rounding counterexample;
vector storage requires BF4 CUDA AST/IR layout producer repair (actual alignment2, modeled8), plus
BF3 CUDA6/2 versus LLVM8/8 handling. General BF arithmetic/comparison, integer/Half/double casts,
explicit vector Select, storage/resources/aggregates/globals/parameter groups and external CUDA BF
helper interoperability remain excluded. Material-driven runtime work remains deferred because
bindings, textures/LUTs, inputs and output oracle are absent; reconsider before each next slice.

## Latest bounded research

Research243 is independently accepted; no production support was admitted. Read
[plan243](plan.slice-243-fp8.md), [report243](report.slice-243-fp8.md),
[evidence243](semantic-evidence.slice-243.json) and [FP8 contract](../docs/design/nvvm-fp8-scalar-contract.md).
Fourteen qualified SM80 GPU controls pass170,030 complete words: format-distinct i8 scalar/internal
helper transport and CUDA12.9 RNE/SATFINITE Float32 casts, including actual Slang NVRTC lowering.
The small smoke gate passes4. All40 source paths,12artifacts,560inputs and prior research indices
remain exact. Full242/targeted233/cadence0 and1692/1651correct/41unresolved/16resolved histories stand.

Independent parent acceptance verifies all180 indexed artifacts,19 primary sources and all170,030
qualified runtime words. Nine parent artifacts retain the independent grid, runtime, producer and
provenance audits. This research is ready for the authorized local commit.

The measured next action is shared FP8 finite/subnormal producer repair before backend admission:
literal E4M3(256) folds to448 and minimum subnormals misfold on the unchanged compiler. Preserve
existing unit-tested shared overflow policy; its NaN/Inf behavior differs from CUDA SATFINITE and
needs a separate policy decision. Known1.25 scalar transport/literal/bitcast remains a smaller future
admission boundary; qualified runtime casts need format-specific semantic planning. Dynamic-object
aggregate/storage, vectors and external ABI remain separate. Material runtime was reconsidered but
still lacks bindings/textures/LUT/input/oracle; support/correctness remains the cadence override.

## Checkpoint and proof

- Full242:452 frozen identities/1,356cells and111 old discovery identities/333cells plus one
  new fixture/3cells. Total1,692cells/1,651correct/41unresolved;16resolved histories. All1,646 old
  correct cells survive. The only two five-field deltas are frozenBF16 O0/O3 preflight→correct.
  Each entire former failure record, including its diagnostic history, is retained in the resolved
  ledger. Discovery333oldcells are exact; no baseline reset or inherited final-source cells.
- Final smoke4, fixture3, ordinary exported helpers3, units484 plus one old Windows skip, toolkit18,
  runnercontracts6 and material6 compile/assembly cells pass. Material runtime/performance remains
  blocked by the absent application contract; support/correctness work is the explicit cadence override.
- Prototype3sourceNVRTC+6rawLLVM and production3sourceNVRTC+6directNVVM launches check195,858 full
  words (12,240active/183,618preserved). Each width has680records: original679payloads unchanged plus
  one reversed cancellation case exposing both contraction and FP32 accumulation. Worker and parent
  independent rational oracles agree. BF16 NaN output expectations check classification only.
- Six final direct linkedIR/PTX controls retain exact BF signatures and2N BF16 FMA instructions.
  Real-provider tests qualify both dialects and reject malformed/format/lanes/arity/physical-type
  mismatches. The fixture passed acceptedNVRTC and failed accepteddirect modes at dot2 before edits.

## Environment and evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86,
LLVM14, providerABI40 and matching RelWithDebInfo. Use inspected slice-203-env.sh and local build
skill. Four CPU workers maximum total, sequential GPU suites and30-minute bounds.

Compiler executable SHA256 `79f46ef0dba116bdfdce8a03b40f7d3bb2c7b481529b452b483d650cf0958e43`. Provider SHA256 `c0522674424c86dbc9444b2abc202c97146a6b41e3a9179d95d34ec9fe1b0773`. All40 final source paths,
12 artifacts (including rebuilt shared compiler library) and560 runtime inputs match before every
gate and afterward; all559 old inputs are exact.

Raw roots `build/nvvm-loop/slice-242-before` and `slice-242-after` retain complete evidence. See
`audit.json`, `replay-audit.json`, `frozen-comparison.json`, `trace/shape-proof.json`,
`dot-replay/results.json`, `artifact-index.json` and compact validation. The generator-typo attempt
is retained separately. All117 research238,132 research240 and88 accepted241 indexed artifacts remain
immutable. No GPU loss, driver/system change, reboot, worker commit or push occurred. Independent parent acceptance verifies707 compact and1,341 total evidence references before its seven
own references, all129 current indexed artifacts, exact corpus/history preservation and195,858
replay words. See `parent-acceptance-audit.json`, `parent-prototype-oracle-audit.json`,
`parent-public-oracle-audit.json` and `parent-countermodels-audit.json`.

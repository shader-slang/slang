# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before resuming the loop.

## Current state and next action

**Research240 is accepted after independent parent review.** Read
[plan240](plan.slice-240-bf16-vectors.md), [report240](report.slice-240-bf16-vectors.md),
[semantic evidence240](semantic-evidence.slice-240.json) and the
[vector contract](../docs/design/nvvm-bf16-vector-contract.md).
No production source, binaries, providerABI38, corpus or input changes.

The next bounded implementation should admit BF16 vector widths2/3/4 as register values and
by-value internal helper arguments/results, construction/splat/extraction and helper branch/phi selection and matching-lane-count
Float32 component conversions. Existing bitcast lowering handles source uint32/BF2, uint64/BF4 and
ushort3/BF3 transport. Preserve the distinct BF16 semantic descriptor; do not widen the IEEE float
or generic numeric classifiers. A full checkpoint is required for shared type/provider changes.

Leave new local-pointer/storage admission separate. Raw component-array locals work, but CUDA BF3
has6-byte size/alignment2 while LLVM BF3 vectors allocate8/alignment8; CUDA BF4 alignment2 differs
from LLVM vector alignment8. BF2 arrays lose its native alignment4 unless explicitly aligned.
The BF4 CUDA layout producers also currently compute alignment8, contrary to the prelude's2; fix
that producer/model issue before external storage support, never bypass downstream layout checks.
Dot and exact integer construction remain separate research238 contracts. Frozen scalar-bf16 is
still unresolved; vector success alone cannot resolve its dot. Material6 compile/assembly evidence
is inherited from239; missing binding/input/oracle semantics justify this research cadence override.

Latest accepted implementation/full checkpoint:239; latest targeted acceptance:233. Implementation
slices since full:zero. Accepted source base946f3f3b1dfb6ce2e468afdd707aced032843e2e.
No push is authorized. Research240 is ready for its authorized local commit.

## Checkpoint and proof

- Full239:452 frozen identities/1356cells,109 old discovery identities/327cells plus scalar fixture3.
  Total1686cells/1643correct/43unresolved/14resolved histories. All1640 old correct cells preserved.
  See [full validation239](runtime-validation.slice-239.json) for complete immutable failure histories.
- Research240 freshly reruns smoke4 and exact frozen BF16 three-cell inventory. NVRTC is correct;
  direct O0/O3 retain `helper function parameter: vector<BFloat16,4>` and all239 outcome fields.
  Other1683 corpus cells, units482+existing Windows skip, toolkit18, contracts6, scalar fixture3 and
  material6 are explicitly inherited. No support delta, corpus additions or cadence reset.
- Three sourceNVRTC and six rawLLVM O0/O3 vector controls compile/assemble/run. Each run uses73190
  accepted records; everyBF16 payload and all7654 additional Float32 boundary records appear in
  each lane. Dynamic helpers and local roundtrip survive in IR/PTX. All13,613,340 active output words
  and18,004,749 input/inactive/header words pass:31,618,089 total. RawLLVM success is not production
  direct-Slang vector support. Six generated direct controls reject helper vector result types.
- Exact LLVM14 type queries and seven CUDA assertions measure size/alignment/wrapped offsets.
  Narrowing NaNs remain classification-only; SM80 widening is exact high-word transport.
- All37 tested sources,12 artifacts and558 inputs match239 before/after; all117 indexed238 artifacts
  remain unchanged. Worker independent audit and parent complete-buffer oracle agree.

## Environment and evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2,
NVRTC12.9.86, LLVM14, providerABI38, matching RelWithDebInfo. Use inspected slice-203-env.sh and
local build skill; its optimized path overrides the base environment's staleDebug selection.
Four CPU workers maximum total, sequential GPU suites,30-minute bounds, no compiler build here.
Compiler SHA256 a6cd5bd057defd8fc896ebd75813d8b7e5737fe33a095e20840fc73b72233412.
Provider SHA256 cefb3cd3cb44fb0d2c6a201f210ea3c98e1913c2fcac554ea5ad912d6afbcfd7.

Raw root `build/nvvm-loop/slice-240-bf16-vectors` retains scripts, commands, source/IR/PTX,
full buffers, layout attempts, identity captures, audit and artifact index. Slice239 and research238
are immutable. Layout probe setup failures are retained and explained; no production workaround,
weakened oracle, edited executing script, GPU loss, system change, reboot, worker commit or push.

Parent acceptance verifies252 unique evidence references, all132 indexed raw artifacts,12 primary
source hashes, unchanged37 source/12 artifact/558 input identities, and all31,618,089 output words.
See `slice-240-bf16-vectors/parent-acceptance-audit.json` and `parent-oracle-audit.json`.

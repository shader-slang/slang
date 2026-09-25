# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before starting or resuming the loop.

## Current state and next action

**Slice 223 is accepted: CUDA unsigned firstbithigh preserves unsigned sign-bit inputs.** Read the
[completed plan](plan.slice-223-firstbithigh-fix.md), [five-part report](report.slice-223-firstbithigh-fix.md)
and [full result manifest](runtime-validation.slice-223.json). Signed-negative complement now belongs
to I32_firstbithigh; U32 uses the original word. Signed32/64 and unsigned64 behavior is preserved,
and the direct provider is unchanged. The independent research222 replay passes all 18 executions
and 6,912 words, including the 24 previously wrong CUDA source results.

**Next action: research224 isolates the generated KernelContext pointer preflight in masked-prefix
min/max.** Two frozen workloads still reject a helper parameter
`Ptr<KernelContext, addressSpace=1, access=0, operands=4, layout=DefaultLayout>`. Trace the producing
lowering and consuming support check; identify a minimal runnable source with the same boundary,
retain NVRTC output and exact NVVM diagnostic, and audit whether the shape is canonical. Do not
widen pointer admission merely to expose another unsupported instruction. Keep independent FP64
prefix semantics, quad reconvergence and vector-by-value shuffle work separate.

No push is authorized. Material runtime still needs application bindings, texture/LUT/input and
output oracle. Fresh-context delegation remains at the app's agent-thread limit; local work uses
WORKFLOW's fallback and does not imply independent worker review.

Latest full checkpoint and implementation: 223. Latest targeted implementation: 221.
Implementation slices since full: zero. Recent implementation history covers FP64 source reductions
and the demonstrated CUDA bit-index correctness fix; correctness work took priority over material
execution, which lacks its application input contract. Discovery capacity is 50 through 128, with
102 current identities.

## Checkpoints and evidence

| Area                        | Record                                                                                                                     | Interpretation                                                                      |
| --------------------------- | -------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| Slice 223 accepted full     | [Manifest](runtime-validation.slice-223.json), [frozen](census.slice-223.tsv), [discovery](discovery-census.slice-223.tsv) | All 1,662 cells fresh: 1,611 correct and 51 known failures. Latest full checkpoint. |
| Slice 221 accepted targeted | [Manifest](runtime-validation.slice-221.json)                                                                              | FP64 min/max admission; both resolved frozen cells freshly preserved in 223.        |
| Slice 220 accepted full     | [Manifest](runtime-validation.slice-220.json)                                                                              | Historical full checkpoint before FP64 admission and CUDA bit-index correction.     |

Frozen remains 452 identities/1,356 cells; discovery has 102 identities/306 cells. Every one of the
1,659 old runtime cells preserves all five stable outcome fields, and the three new cells pass.
There are no missing, extra, duplicate or inherited runtime cells. All 549 old runtime input hashes
and 101 old discovery manifest rows are unchanged. Historical healthy denominators 427/72 stay fixed.
All 51 open first-known failure records and six resolved histories are preserved exactly except for
fresh evidence references.

Fresh final gates: focused 4/4 plus the existing Windows-only skip, GPU smoke 4/4, units 478/478 plus
the same existing skip, toolkit 18/18, discovery contracts 6/6, research 18/18 and material
compile/assembly 6/6. The skipped fixture is nvvmSlangIntegerBitHelpersRequestTypedOperations; its
real-builder neighbor and all three new GPU modes pass. An initial summary count was corrected
from five requested selectors to four executed passes plus that skip. Both corpus runners return
two for known failures; structured outcomes decide acceptance. No material runtime claim is made.

## Unresolved failures and limitations

- All 51 open failure records and six resolved histories retain first-known evidence and reproduction.
- Quad reconvergence, prefix-min/max
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

Tested base `409ab717d05f1bbabfa24f8df83ce67cf648b47b` plus the CUDA helper/fixture changes.
Compiler SHA256 `01e06def851b6228dea63d2bbb18cb4c3167ea89542d542623ea79e9d6f3258d`.
Provider unchanged `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Raw evidence: `build/nvvm-loop/slice-223-before` and `slice-223-after`, including exact research222
replay and the focused-count reporting correction. Parent verified 138 evidence references,
24 tested source hashes, 12 artifact hashes and 550 runtime input hashes.
No GPU loss, driver change, reboot or push. Local parent review/acceptance follows the recorded
fresh-context delegation limitation.

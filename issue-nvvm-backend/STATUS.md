# NVVM backend status

Read [WORKFLOW.md](WORKFLOW.md) before resuming. The autonomous loop is authorized; the parent owns
independent acceptance and local commits. No push is authorized.

## Slice249 is accepted

Independent parent acceptance passed for bounded FP8 scalar transport with provider ABI41. Read
[plan249](plan.slice-249-fp8-scalar-transport.md),
[report249](report.slice-249-fp8-scalar-transport.md),
[validation249](runtime-validation.slice-249.json), and the
[FP8 contract](../docs/design/nvvm-fp8-scalar-contract.md).

Distinct E4M3/E5M2 descriptors use physical i8 for registers and internal by-value helper values.
Finite canonical literals, signed/unsigned8 and cross-format bitcasts, selects and phi transport are
qualified. Runtime numeric casts/arithmetic, nonfinite literals, FP8 vectors/storage/pointers/resources/
aggregates/external helper ABI, dynamic objects and BF vector storage remain excluded.

All final gates use base `7263a71a8761f61ee04eede22d69c75dab4d77d8` plus the recorded249 diff:
118 source/generated/test snapshots,12 artifacts,562 runtime inputs. Compiler library SHA256
`ce403e533104baa25276ec0dac28af1991bc60c6bd31f356fe92831897f06d9e`; provider SHA256
`5fe0b977e22b80acc5ee39147c69510a01c09563354a1a67bd9573d1cda1aeab`.

- Runtime4, focused6,512 main units plus one supplemental literal unit (one inherited skip),
  toolkit18, runner contracts6 and complex6 compile/assembly pass. Exact511 old focused unit pass
  identities survive alongside two new units.
- Frozen452/1356 preserves all1345 prior passes and resolves exactly the two direct folding cells.
  Original full output is58,61,16288 followed by nine zeros in all modes; CHECK prefixes are unchanged.
- Discovery114/342 preserves all339 old outcomes exactly and adds three passing cells from one
  distinct fixture. Combined1698cells/1659correct/39unresolved; all1654 old passes survive.
  All41 old failure histories remain, with two newly resolved plus16 previously resolved histories.
  Texture and column-major wrong-output baselines are unchanged.
- Six supplemental GPU launches cover every byte in both formats, both branch choices, loop-carried
  FP8 phi, signed-byte internal helper arguments/results and cross-format transport. Whole-buffer
  audit checks10866 final words plus1036 before words, with zero mismatches.

Latest accepted implementation and full checkpoint is249; latest targeted acceptance is233;
implementation cadence0. Rolling implementation history is244 finite FP8 producers,246 material
compile-time improvement,249 scalar FP8 transport. Parent audit independently verifies all old
outcomes and histories, exact output expectations, gate identities, unit IDs and evidence hashes.
No timing or material runtime claim is made. Material bindings/textures/LUT/input/output oracle
remain unavailable.

## Next bounded action

Select a bounded correctness fix for the recorded pre-existing BF16 negative constant boundary.
`pack(BFloat16(-1.25f))` through a non-inline helper rejects `canonical BF16 constant bits`: the
unchanged materialization branch passes unsigned0xbfa0 to the provider's signed-in-width i16 constant
API. The same compile-only diagnostic is reproduced with the preserved accepted244 compiler/library
and the249 candidate under `slice-249-after/next-bf16-constant`. No BF16 fix belongs to249; stop at this
minimal producer/consumer handoff before selecting the next slice.

Shared FP8 overflow still differs from CUDA SATFINITE. General runtime FP8 casts were researched243
but are not admitted here. Dynamic-object/aggregate storage remains an independent boundary.

## Accepted historical evidence

[Research248](report.slice-248-texture-contract.md) and
[semantic evidence248](semantic-evidence.slice-248.json) retain the original texture/mip failures;
they do not establish a new metadata ABI. [Research247](report.slice-247-column-major.md) retains the
original column-major mismatch and separately proves compact CUDA stride12 versus graphics stride16.
Neither research slice changes the frozen oracle or current corpus IDs.

[Implementation246](runtime-validation.slice-246.json) and
[report246](report.slice-246-ast-subtype.md) retain the accepted AST predicate inlining and measured
material compile-time improvement. [Implementation244](runtime-validation.slice-244.json) repairs
finite/subnormal FP8 producers while preserving overflow policy. [Research243](semantic-evidence.slice-243.json)
qualifies the historical scalar transport/conversion controls. Old raw indices243–248 are unchanged;
249 verifies180/58/528/2830/158/158 indexed artifacts against their original snapshots.

## Environment and evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86,
LLVM14, providerABI41, matching RelWithDebInfo. Inspect/source `build/nvvm-loop/slice-203-env.sh` and
follow the local slang-build skill. At most4 CPU workers, sequential GPU suites and30-minute bounds.

Raw roots `build/nvvm-loop/slice-249-before` and `slice-249-after` retain exact before/final source
snapshots, the failed first constant-argument attempt, all outputs, commands, identities and artifact
index. No GPU loss, driver/system change, reboot or push occurred.

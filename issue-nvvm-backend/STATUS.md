# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before resuming the loop.

## Current state and next action

**Research247 is independently accepted.** Read
[plan247](plan.slice-247-column-major.md), [report247](report.slice-247-column-major.md) and
[evidence247](semantic-evidence.slice-247.json). The original column-major discovery ID still has
three runtime mismatches (`11,1` actual versus `11,22` graphics oracle); all are retained. Eighteen
independent CUDA controls prove column stride12 and row stride8 in NVRTC O3/NVVM O0/O3, with six
PTX assemblies and runtime4 passing. The fixture's ignored-major comment is stale for this shape:
the mismatch comes from graphics column stride16 input packing. No compiler/runner/fixture change,
new registered ID, support unlock, full checkpoint or implementation-cadence increment.

After the local commit, re-rank remaining boundaries. The texture-dimension mismatch is the
remaining unqualified wrong-output case; narrow FP8 admission and arbitrary RequirePrelude remain
separate support candidates. A future explicit packed CUDA matrix contract must preserve the original ID/oracle and
complete a full checkpoint for corpus/runner changes. No general host-byte repacking is proposed.
Latest implementation/full remains246, targeted233, implementation slices since full zero. All41
unresolved and16 resolved histories remain exact246 records. Tested HEAD for247 is
`739ead0d725f1075b4d382ec531ebe9070e60ea1`;117 source,12 artifact and561 input hashes match246.

**Implementation246 and its full checkpoint are independently accepted.** Read
[plan246](plan.slice-246-ast-subtype.md), [report246](report.slice-246-ast-subtype.md),
[validation246](runtime-validation.slice-246.json), [timing246](timing-evidence.slice-246.json)
and the [material compile-time design note](../docs/design/nvvm-material-compile-time.md).

The exact existing `SyntaxClassBase::isSubClassOf` body now lives in its class definition, exposing
it to compiler inlining. Generated hierarchy, canonical tags, cast policy, producer and backend
admission remain unchanged. No new production helper or fallback. The direct-tag alternative was
deferred because this minimal change meets the predeclared gate.

Latest accepted implementation/full checkpoint:246. Latest targeted acceptance:233. Implementation
slices since full:zero. Tested source base `c29b9b7158ab069141476761f5585c26d3cf7460`. Accepted commit `739ead0d725f1075b4d382ec531ebe9070e60ea1`; no push is authorized. Rolling history:242dot/244FP8producerfix/246materialcompiletime.

Material runtime still lacks bindings, textures/LUTs, inputs and an output oracle. The accepted
compile-time improvement makes no kernel-performance claim. Shared FP8 overflow policy and BF
vector storage remain separate decisions. Parent247 independently verified all144 control values,
18 executions, six trace/assembly pairs, three exact old outcomes, 158 indexed artifacts, 12 primary
source snapshots and166 compact references before its own two audit references. See parent-audit
under `build/nvvm-loop/slice-247-research`.

## Accepted performance and correctness evidence

Two opposite-order paired rounds retain264 compiles and24 assemblies, all byte-identical to244/245.
Semantic medians fall18.43–25.56%; the sum of six wall medians falls9.43%. Five pooled wall medians
improve11.38–11.83%; sample/NVVM O3 is0.066% slower pooled amid substantial dispersion, while both
round medians improve. All predeclared gates pass. No timings were excluded or rerun; the cause of
variation is not established. The compiler library shrinks320656 bytes and its ELF `.text`324816 bytes.

The exhaustive proof covers702 tags,492804 subtype pairs including abstract classes, and636 real
ASTBuilder objects. It checks four cast overloads, nulls, const types, pointer roundtrips, default/null
metadata and existing DeclRef restrictions. Invalid-tag assertion code is unchanged; no invalid tags
were executed against optimized objects.

- Runtime4, full compiler units1045 passed/13 ignored, semantic regressions1052 passed/77 ignored,
  toolkit18, runner contracts6 and material6 compile/assembly cells pass.
- All1058 unit identities match before. All1044 original passes survive. The sole skip-to-pass comes
  from a missing generator in the copied baseline layout; a focused old-binary control passes with
  the unchanged generator restored. All1129 semantic identities/outcomes are exact.
- Frozen452/1356 and discovery113/339 preserve all1695 old five-field outcomes:1654 correct,
  41 known unresolved and16 resolved histories, with no additions or deltas. The original failure
  histories and reproductions remain in validation246; no baseline reset.
- Parent independently verifies paired statistics, exact production-body relocation, all final
  identities,117 source snapshots,2830 indexed raw artifacts and918 references including historical245
  before its six own audit references. Older research/checkpoint indices remain immutable.

Historical244 literal/helper/dynamic replay/trace controls retain their original source and artifact
identities. Earlier BF16/raw LLVM/export controls also remain historical. Registered runtime corpus,
compiler units, semantic regressions, exhaustive proof and material support checks are fresh246.

## Prior qualifications and remaining boundaries

[Research245](timing-evidence.slice-245.json), committed at
`c29b9b7158ab069141476761f5585c26d3cf7460`, measured the hotspot and defined the accepted gates.
All528 indexed artifacts,15 source snapshots and four parent audit artifacts remain intact.
[Implementation244](runtime-validation.slice-244.json), committed at
`8d53504112617efda0e3446f7b3117bcc2f77fd2`, repaired shared finite/subnormal FP8 producers.
[Research243](semantic-evidence.slice-243.json) qualified scalar/internal-helper FP8 transport and
CUDA RNE/SATFINITE casts; its19 primary-source snapshots remain intact. Literal overflow policy
still differs from CUDA runtime SATFINITE. FP8 vectors/storage/aggregates/external ABI are separate.
ABI40 BF16 source-ordered dot remains accepted. BF vector storage requires its recorded layout repair.

## Environment and raw evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2/NVRTC12.9.86,
LLVM14, providerABI40 and matching RelWithDebInfo. Inspect/source slice-203-env.sh and follow the local
slang-build skill. At most4 CPU workers total, sequential GPU suites and30-minute bounds; timings
run without competing builds/benchmarks.

All final gates capture identical117 source/generated/test paths,12 artifacts and561 unchanged runtime
inputs. Compiler source/artifacts match the measured candidate; later standalone-test refinements do
not rebuild the compiler. Compiler executable SHA256
`b9e87811c263f131cf1372b307bd60acdcc51993aaab2af91a0cfd03462d10a1`;
compiler-library SHA256 `36eeb7034dcdc8501b11f593f0e3a7af29ee977f574488018b6f0c1f0cc0dd57`;
provider SHA256 `c0522674424c86dbc9444b2abc202c97146a6b41e3a9179d95d34ec9fe1b0773`.

Raw roots: `build/nvvm-loop/slice-246-before`, `slice-246-prototype-inline`, `slice-246-after`.
See parent-timing, parent-corpus and parent-metadata scripts/JSON, artifact-index.json, source-snapshots.json
and the compact manifests. The corrected test-assumption compile failure and supplemental baseline
generator control are retained explicitly. No GPU loss, driver/system change, reboot or push occurred.

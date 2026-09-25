# NVVM development handoff

Updated 2026-09-25. Read [WORKFLOW.md](WORKFLOW.md) before resuming the loop.

## Current state and next action

**Slice239 is accepted after independent parent review.** Read
[plan239](plan.slice-239-bf16-scalar.md), [report239](report.slice-239-bf16-scalar.md),
[full validation239](runtime-validation.slice-239.json) and raw `slice-239-after/audit.json`.
ABI38 gives scalar BF16 a distinct semantic format with physical i16 values, constants, helper/local
storage, bit transport and exact SM80 Float32 conversions. No frontend/library/runner changes.
All IEEE Half classification remains unchanged; integer/Half/double casts, arithmetic, vectors,
aggregate/resource BF16 storage remain outside this scalar contract.

Latest accepted implementation/full checkpoint:239; latest targeted acceptance:233. Implementation
slices since full:zero. Rolling feature history:235 quad helpers,237 clocks,239 scalar BF16.
Slice239 is research-backed correctness/support work. Material runtime remains deferred because its
binding/input/oracle contract is absent; its six support cells were freshly reassessed. Research238 and accepted236/237 remain
immutable. The next slice should separately qualify the canonical BF16 vector transport/FloatCast
boundary revealed in frozen scalar-bf16, followed by source-ordered dot; do not infer either from
scalar correctness. Exact integer construction is also separate due measured double rounding.
The accepted slice is ready for its authorized local commit. No push is authorized.

## Checkpoint and proof

- Frozen452 identities/1356cells selected explicitly from immutable slice195; discovery109 old
  identities/327cells plus scalar fixture3. Total1686 fresh cells/1643correct/43unresolved/14resolved
  histories. All1640 old correct cells preserved; no missing/duplicate/extra cells or baseline reset.
- Compare classification, return code, complete execution counts, diagnostic and canonical shape.
  Only two original frozen BF16 direct diagnostic texts move to the next valid vector boundary;
  neither cell is resolved. Their complete prior failure records and all43 first-known/reproduction
  histories remain, as do all14 resolved histories.
- Final smoke4, fixture3, units482 plus one existing Windows-only skip, toolkit18, contracts6 and
  material compile/assembly6 pass. One new real-provider unit checks semantic exclusions and both
  LLVM dialects; six frontend-valid negatives reject before provider discovery.
- Readable fixture is frozen from revised old-compiler before-proof: NVRTC passes; both direct modes
  reject E52017 helper result BFloat16. Preserved compiler library hash verifies accepted237 bytes.
  Provider had already rebuilt ABI38 but was never discovered for rejected before cells. Do not
  interpret that mixed setup as ABI38 success. Final fixture adds input-driven branch/phi selection;
  emitted IR also proves i16 helper args/results and local alloca/load/store alignment2.
- All73190 accepted research238 records replay through scalar-only public NVRTC/O0/O3 projection and
  accepted raw-i16 O0/O3 controls. Original inputs/oracles unchanged, columns4/5/6 active,7/8 inactive.
  Five assemblies and5,855,205 complete words pass; separate before launch also audited. No universal
  NaN payload claim: narrowing classification, SM80 expansion exact bits. Constants preserve the
  canonical IR value through the same core helper used by its producer.

## Unresolved limitations

Frozen BF16 vector transport/casts/dot are not supported by this slice. Integer16842753 proves why
ordinary int→Float32→BF16 is not an exact constructor. Half/double/general BF→integer, arithmetic,
BF16 aggregates/resources, FP8/dynamic dispatch and arbitrary prelude/texture work remain separate.
The matrix mismatch remains CUDA's documented target-wide ignored column-major limitation.
Material6 freshly pass compile/assembly only; bindings/textures/LUT/input/oracle remain absent,
so no material runtime/performance claim. The source shader is untouched.

## Environment and evidence

Native Ubuntu24.04, branch nvvm-backend, L4SM89 driver580.126.09, targetSM80, CUDA12.9.2,
NVRTC12.9.86, LLVM14, providerABI38, matching RelWithDebInfo. Use inspected slice-203-env.sh and
local build skill. Four CPU workers total, unit servers2, sequential suites,30-minute bounds,
CMAKE_BUILD_PARALLEL_LEVEL=1. Tested base d7732c6ba2e2978811b628ab5740bdd25e9a273d plus recorded patch.
Compiler SHA256 a6cd5bd057defd8fc896ebd75813d8b7e5737fe33a095e20840fc73b72233412.
Provider SHA256 cefb3cd3cb44fb0d2c6a201f210ea3c98e1913c2fcac554ea5ad912d6afbcfd7.
Each final gate matches37 source paths,12 artifacts,558 inputs; all557 old inputs remain exact.

Raw roots `build/nvvm-loop/slice-239-before` and `slice-239-after` retain commands, source/IR/PTX,
complete replay buffers, identities, exact comparisons and audits. Initial unit-negative failure is
retained under attempt1: two ambiguous frontend probes were removed from the backend matrix;
provider rejection coverage remains. Final unit-only rebuild and every gate followed correction.
Incomplete attempt2 was safely stopped for dynamic Select test strengthening, then every final gate
restarted. Neither incomplete checkpoint is counted as passing evidence.
No production fallback, source/oracle weakening, edited executing scripts, GPU loss, system change,
reboot, worker commit or push. Parent independently checks all replay words and full acceptance.

Worker audit verifies693 compact references and immutable research238117/research236241 artifacts.
Independent parent acceptance verifies699 unique compact references and1,073 including accepted
research artifacts, exact outcomes and complete histories, all37 source/12 artifact/558 input hashes,
and all5,855,205 final replay words. See `slice-239-after/parent-acceptance-audit.json` and
`parent-oracle-audit.json`. Full checkpoint239 resets implementation cadence to zero.

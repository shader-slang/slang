# Establish the CUDA texture-dimensions mismatch contract

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow requires this completed plan and report to be committed; raw evidence remains ignored under `build/`.

## Purpose and Observable Result

Establish why `compute/texture-get-dimensions.slang#discovery-1` has one existing NVRTC wrong-output result and two direct NVVM mip-query GenericAsm stops. Preserve its source, resource inputs, oracle, identity and all historical outcomes. Independently derive the original API expectation and actual CUDA behavior with focused controls. Close research-only if a resource metadata/ABI extension would be required; propose any bounded compiler change to the parent before implementation.

## Progress

- [x] 2026-09-25: Read AGENTS, plans, workflow, status and local slang-build skill.
- [x] 2026-09-25: Verify clean branch base `7d8cd100aaf29d2334876655fc7c8eee0249a691`.
- [x] 2026-09-25: Verify matching accepted identities and small runtime4 gate.
- [x] 2026-09-25: Reproduce three original outcomes and derive producer/consumer/API contract.
- [x] 2026-09-25: Run independent geometry/mip/array controls and retain traces.
- [x] 2026-09-25: Record precise boundary, completed report, evidence and handoff.

## Surprises and Discoveries

Fresh original outcomes are exact246. Seven missing mip counts are masked by float rounding; only three array counts produce visible mismatches. Integer controls expose both counts and ignored LOD. Actual RHI creates full chains and treats arrayLength1 as non-array, including the declared cube-array fixture. Independent driver objects prove LOD1 width and shape-specific layer counts work. Metadata query cubins assemble and retain the same probe symbol but function lookup returns500; their undefined weak descriptor-size symbol is correlated evidence, not a proven driver cause. Surface query width units are elements. A macro-shadow compile failure and combined unavailable-query launch failure were isolated and retained. The initially mislabeled surface field was corrected with prior source/output retained.

## Decision Log

2026-09-25, worker: prioritize the remaining unqualified runtime mismatch over FP8 admission and arbitrary RequirePrelude. Research first; no speculative metadata ABI, source rewrite, oracle weakening or failure reclassification.

2026-09-25, worker: close research-only. Reject admitting incomplete mip helper, inferring levels from base width, opaque handle inspection and automatic metadata ABI invention. Nonzero LOD/layer geometry is available, so do not claim universal unavailable queries. Parent agreed research-only; a new implementation needs a separately reviewed gate.

## Outcomes and Retrospective

Research independently accepted by the parent. Original one mismatch/two preflight failures unchanged;12 Slang executions match246 independent current-CUDA values, two expected plain1D-array stops retained. Five trace assemblies, three isolated assemblies, nine independent host/geometry/LOD resources and runtime4 pass. This does not make the current helper API-correct. Full246 and all41/16 histories remain inherited; no production/test/runner/oracle change, support unlock or cadence increment. Next full-API gate is explicit resource/view total-level semantics plus producer-owned query semantics; partial LOD/layer support is a separate bounded proposal.

## Context and Current Pipeline

The original fixture binds 1D/2D/3D/cube and array textures, compares plain dimensions against mip-zero dimensions, and packs width/height/depth-or-array-count/mip-count into uint before writing float. Trace `TextureTypeInfo::writeGetDimensionFunctions`, CUDA helper assembly, direct NVVM resolver/provider, render-test texture descriptions and CUDA resource creation. Distinguish underlying mip allocation from sampler-visible clamp and unavailable device-query fields.

## Scope and Non-Goals

Only this texture contract and neighboring accepted texture-dimensions control. No compiler, harness, fixture, discovery inventory or ABI mutations without parent agreement. No driver/system changes, reboot, commit or push. Preserve full246 1695 cells,1654 correct,41 unresolved,16 resolved histories; latest targeted233 and cadence0 remain.

## Architecture and Invariants

The original graphics oracle and CUDA helper contract remain separately visible. Device tex/surface handles are opaque; resource descriptors are host metadata. An independent expected output must be derived before comparing modes; NVRTC agreement alone is insufficient.

## Interfaces and Dependencies

Native Linux RelWithDebInfo, ABI40, CUDA12.9, targetSM80 on L4. Source `build/nvvm-loop/slice-203-env.sh` (corrects stale Debug setup). Match 117 sources,12 compiler/library/provider artifacts,561 runtime inputs against246. Max4 CPU workers total, sequential GPU suites,30-minute timeout. Use embedded render-test through the existing research server adapter.

## Milestones

1. Capture initial identity and device state; run runtime4 and focused discovery match texture-get-dimensions.
2. Snapshot primary repository/CUDA documentation sources and trace actual resource production. Create additive research fixtures under slice-248-research with distinct dimensions, mip and array counts; run NVRTC O3/NVVM O0/O3 where supported, retain unsupported diagnostics.
3. Document responsible layer and future implementation gate, exact fresh/inherited cells, raw index and final identity.

## Validation and Acceptance

Run `timeout --kill-after=30s 30m python3 extras/validate-nvvm-runtime.py --config RelWithDebInfo --cuda-path /usr/local/cuda-12.9 --architecture 80 --output build/nvvm-loop/slice-248-before/runtime`.
Run discovery with matching bin/provider/architecture, accepted manifest, `--match texture-get-dimensions --keep-mirrors`, output slice-248-before/discovery. Require exactly3 original outcomes and compare exact five fields to246. Run accepted tests/cuda/nvvm-texture-dimensions.slang and independent controls. No full1695 replay for research-only with exact identities. Full checkpoint required for any compiler/library/provider/ABI/runner change.

## Failure and Recovery

Stop GPU dispatch on device loss; retain failed attempts without overwrites. Keep independent blockers bounded after exact minimal handoff. Research controls are not new corpus support. Preserve all accepted raw files.

## Artifacts and Hand-Off

Raw `build/nvvm-loop/slice-248-before` and `slice-248-research`; completed plan, five-part report, durable design note, STATUS and `semantic-evidence.slice-248.json`. Parent owns acceptance and commit after worker explicitly releases checkout ownership.

Final evidence: `semantic-evidence.slice-248.json`; raw acceptance checks14 controls (12 executions,
2 expected stops),246 uint values, five traces/assemblies, three isolated query assemblies,
nine standalone CUDA objects and all original three five-field outcomes. `summarize.py` verifies
matching117/12/561 identities and exact41/16 histories. Raw index currently158 files,16 local
primary-source snapshots plus official web snapshots. Parent owns the final audit and commit.

Parent acceptance: all246 uint values, three original five-field outcomes, nine host mip tables,
27 base-query values, nine LOD values, four surface values,158 indexed artifacts,16 source snapshots
and171 compact references verify before the two parent references. Exact117/12/561 identities and
41/16 histories match246. See parent-audit.py/json; local commit is owned by the parent.

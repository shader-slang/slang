# Support integer and vector texture dimension queries

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow explicitly requires committing completed
plans and reports; the parent owns acceptance and commit. Base: ff3c679909428fd963787de74abc5cfb72faa6f3.

## Purpose and Observable Result

Canonical non-mip CUDA GetDimensions on supported read-only textures will work for integer/vector
texels and signed or unsigned 32-bit dimensions. Real texture fixtures verify known dimensions at
NVRTC O3 and direct NVVM O0/O3, including the material's UInt64 descriptor and int2 output path.
This does not claim full material runtime support.

## Progress

- [x] 2026-09-24: Read workflow, status, optimized checkpoint and slice-203 report/evidence, build skill.
- [x] 2026-09-24: Identify both scalar-float texel restriction and unsigned-only output restriction.
- [x] 2026-09-24: Byte-identical focused fixture: NVRTC passes, direct O0/O3 reject E52017; runtime 4 healthy.
- [x] 2026-09-24: Compiler/provider admission built; final focused 14/14 and runtime 4 pass.
- [x] 2026-09-24: Units 473 plus one Windows-only skip and toolkit 18 pass; all six complex cells reassessed.
- [x] 2026-09-24: Helper/input-shape self-review and diagnostic-producer audit complete; five-part report drafted.
- [x] 2026-09-24: Full checkpoint and exact comparison complete; durable manifest/report/STATUS ready for parent review.

## Surprises and Discoveries

Provider `_isTextureOperationSupported` repeats the scalar-float query restriction. Consequently
this is a provider support-contract change and requires full acceptance, not targeted acceptance.
The provider uses nvvm_txq_width/height/depth from the texture handle; element type is not consumed
by query emission. Dimension stores already use the lowered i32 value and 4-byte output pointer.

The first diagnostic run exposed a harness issue: diagnostics require an explicit output path to
force code generation. The unchanged fetch negative also expected the older bare GenericAsm
message; it now matches the existing expanded assembly diagnostic. Neither issue changed compiler
behavior. Final diagnostics explicitly request PTX and all six query boundary lanes pass.

## Decision Log

2026-09-24: Reuse getNVVMSupportedReadOnlyTextureType, existing signed/unsigned i32 predicates,
and the provider's existing 32-bit scalar/2/4 numeric texel classification. No new representation,
ABI, shader reinterpretation, query fallback or shared type lowering. Expand coherent query domain,
not just the exact uint material case. Replace float2 unsupported query test with a still-unsupported
mip query; add positive element-family coverage. Full checkpoint required by provider impact.

## Outcomes and Retrospective

Focused queries now execute correctly in all three modes with identical pre-change fixtures. All
four direct complex cells move to the independent OutParam<mtlx.BSDF> -> BorrowInOutParam<mtlx.BSDF>
call boundary. Both NVRTC material PTX outputs remain byte-identical. The full runtime checkpoint
is complete: 1,611 fresh cells, 1,550 correct and 61 known failures. All
1,547 prior correct obligations and exact prior failure outcomes/diagnostics are preserved; three
new cells pass. No missing/duplicate keys, inherited outcomes or runtime diagnostic changes.
Parent integration review accepted this full checkpoint. The checkpoint resets cadence
to zero; no independent next feature was started.

## Context and Current Pipeline

The unchanged tiled-brass material's render.TextureHandle.resolve_udim constructs Texture2D<uint>
from UInt64 descriptor bits and invokes GetDimensions(dim.x, dim.y), where dim is int2. Standard
library TextureTypeInfo generation creates the canonical GenericAsm helper with two OutParam<int>
arguments and txq.width.b32/height.b32 assembly. `_resolveNVVMTextureDimensionsGenericAsm` recognizes
that exact assembly but rejects the type before creating texture requirements; provider support
independently rejects it. Producer IR is canonical, not a malformed alternative. Queries read
resource geometry independent of sampled texel types; signed/unsigned dimensions share i32 storage.

## Scope and Non-Goals

Only non-mip query admission and regression coverage. Preserve resource classification, shape and
assembly checks, MS/mip rejection, integer sampling exclusions, and CUDA array-size zero behavior.
No independent material blocker, runner rewrite, driver action, broad lowering or performance work.

## Architecture and Invariants

Existing read-only texture classification owns the resource domain. Query resolver owns exact
prelude signature/assembly recognition. Provider owns corresponding operation admission and LLVM
query emission. Keep exact assembly matching and numeric local OutParam i32 validation. No source
syntax reconstruction, arbitrary graph walks, new custom equivalence or shader edits.

## Interfaces and Dependencies

Native Linux L4 SM89, CUDA12.9 target SM80, matching build/RelWithDebInfo tools and provider ABI35.
Use build/nvvm-loop/slice-203-env.sh (overrides Debug paths) and CMAKE_BUILD_PARALLEL_LEVEL=1.
Build: cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server.

## Milestones

1. Add focused real-texture oracle with element families, signed/unsigned outputs and shape/size
   boundaries; record fail-before and hashes on unchanged compiler. Preserve test inputs afterward.
2. Narrow compiler/provider admission change and relevant unit expectation updates; build, format,
   run focused positives and unsupported queries/fetch/sampling negatives.
3. Runtime 4, units 473+skip (plus justified additions if necessary), toolkit 18; full frozen/discovery
   and complex 6. Record next independent blocker only. Compare exact outcomes and finalize artifacts.

## Validation and Acceptance

Affected domain: all supported read-only texture queries, texel families and output signedness;
neighbors: texture load/sample/gather, descriptor/helper transport and excluded shapes/mips.
Full checkpoint trigger is already met by provider support-contract expansion. Fresh runtime cells:
452 frozen identities x3 plus all 84 accepted discovery identities x3, plus eligible added fixture
identities x3. Register new sources only in discovery, no frozen overlap. Six complex support cells,
focused diagnostics/oracle, runtime 4, units473+skip, toolkit 18 also fresh. Inherited outcomes: zero
at final full checkpoint; before references are optimized checkpoint plus slice 203 overlay/addition.
Preserve all 1,547 correct obligations and 61 failures; classify any newly exposed diagnostics without
hiding old failures. Require exact IDs/modes, no duplicates/missing, actual execution counts. Full
manifest loader requires50–100 discovery records; run authoritative full manifest. Baseline
counts 452 frozen/84 discovery, totaling 1,608 runtime cells. Additions tracked separately.
Use bounded 30m suites sequentially and at most four CPU workers. Stop GPU dispatch on device loss.

## Failure and Recovery

Before source edits capture fixtures and matching compiler/provider hashes. Revert drill where
practical; unchanged-source before fixtures suffice if final inputs are byte-identical. On regressions
isolate responsible change or revert, never reset baseline. Independent next blocker is handoff only.
No commit/push/driver/reboot. Parent accepts or rejects bounded change.

## Artifacts and Hand-Off

Raw logs and exact commands: ignored build/nvvm-loop/slice-204-before and slice-204-after.
Durable full per-cell outcomes, comparison/provenance, five-part report, completed plan and STATUS.
Update design only with lasting query semantics. Final source and binaries hashes must match gates.

Parent integration review accepted the compiler/provider admission change, full exact-cell
preservation, final hashes, and retained negative boundaries. Full checkpoint resets cadence to zero.

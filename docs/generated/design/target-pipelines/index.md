---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-02T22:00:00Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: fb0c54d112f6e8200c9f5c0b19aaf2d3e968adf8b0c80f4bbc191766994f4b9f
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Target Pipelines

This page is a navigation hub for the per-target pipeline pages
in `target-pipelines/`, written for compiler developers who need
to pick the right per-target page. Each peer page documents one target's
ordered IR-pass and downstream-tool sequence as a four-phase
control-flow-graph view of the shared orchestrator
`linkAndOptimizeIR` (line 1015 of
[../../../../source/slang/slang-emit.cpp](../../../../source/slang/slang-emit.cpp)).
For an unordered, topical catalog of every IR pass — grouped by
category rather than by execution order — see
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md). For the
capability / profile *model* that decides what a target is allowed
to emit, see [../cross-cutting/targets.md](../cross-cutting/targets.md);
the pages here document per-target pipeline *behavior* only.

## Pages

- [spirv.md](spirv.md) — SPIR-V via the direct-emit path, with the
  spirv-link / spirv-val / spirv-opt downstream chain.
- [hlsl.md](hlsl.md) — HLSL source, with DXC (DXIL) and fxc
  (DXBytecode) downstream.
- [metal.md](metal.md) — Metal source, with the Apple `metal`
  compiler downstream for `MetalLib` / `MetalLibAssembly`.
- [wgsl.md](wgsl.md) — WGSL source, with Tint (WGSL to SPIR-V)
  downstream for `WGSLSPIRV` / `WGSLSPIRVAssembly`.
- [cuda.md](cuda.md) — CUDA C++ source / header, with nvrtc (PTX)
  downstream.

## Shared shape

All five pages obey the **Target-pipeline page contract** in
[../_meta/prompts/_common.md](../_meta/prompts/_common.md) and
decompose their target's invocation of `linkAndOptimizeIR` into
four phases:

- **Phase A — Link and entry-point prep.** Link the per-module IR
  and prepare entry points for legalization, including the
  per-target entry-point-uniform handling. See the per-target page
  for the exact set of passes its target lands on.
- **Phase B — Specialization and type legalization.** Specialize
  generics and resolve target-independent type-legalization
  questions. The big cross-target divergences (existential and
  resource-type legalization, cooperative-vector lowering,
  target-specific wrappers) live here.
- **Phase C — Target legalization, lowering, phi elimination.**
  Run the target-specific legalization driver (where one exists)
  along with shared lowering, then leave SSA via the phi-elimination
  step.
- **Phase D — Emit and downstream tools.** Hand the legalized IR
  to the target's `CLikeSourceEmitter` subclass (or the SPIR-V
  direct-emit path), wrap the result as a downstream-compiler
  input, and run
  the target's external tools.

All five pages therefore carry the same `##` skeleton, in this
order: `Source`, `High-level phase diagram`, the four
`Phase A`–`Phase D` sections, `Conditional gates`,
`Loops in the pipeline`, `Notable passes`, `See also`. There are
exactly two deviations, both deliberate:

- [cuda.md](cuda.md) inserts one extra section,
  `## Adjacent targets`, between Phase D and Conditional gates. It
  exists because `PyTorchCppBinding`, OptiX, and the host-CPP
  targets share several CUDA switch arms without being CUDA, and
  the page needs to say explicitly that they are out of scope.
- [spirv.md](spirv.md) titles its fourth phase
  `## Phase D: IR-to-SPIR-V emit, simplification loop, downstream
  tools`, because SPIR-V is the one target whose *legalization
  driver* runs inside the emit step rather than inside
  `linkAndOptimizeIR` (see the comparison table below).

Every page's `## Conditional gates` section discusses the
content-based gating described under
[Filtering rules](#filtering-rules).

Reading any single per-target page yields the **filtered** view
of `linkAndOptimizeIR` — passes that fire only for sibling
targets are omitted from the diagrams and tables. The shared
orchestrator runs unconditionally for every target; what differs
is which switch arm each target lands in.

## Cross-target comparison

| Target | CodeGenTarget enum values | Phase C entry | Phase D emitter | Downstream tools | Loops |
| --- | --- | --- | --- | --- | --- |
| SPIR-V | `SPIRV`, `SPIRVAssembly` | (no single entry in `linkAndOptimizeIR`; per-pass SPIR-V arms) — the `legalizeIRForSPIRV` driver ([slang-ir-spirv-legalize.cpp](../../../../source/slang/slang-ir-spirv-legalize.cpp) line 3433) runs in **Phase D**, called from `emitSPIRVFromIR` | `emitSPIRVForEntryPointsDirectly` ([slang-emit.cpp](../../../../source/slang/slang-emit.cpp) line 3774) → `emitSPIRVFromIR` ([slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp) line 12469) | spirv-link, spirv-val, spirv-opt | **Yes** — `simplifyIRForSpirvLegalization` and the forward-declared-pointer fixup in `emitSPIRVFromIR` both run to convergence (see note below). |
| HLSL | `HLSL` (plus downstream `DXIL`, `DXBytecode`, and their `*Assembly` variants) | (no single entry; per-pass HLSL arms, e.g. `validateBarrierFlagsForHLSL` in [slang-ir-hlsl-legalize.cpp](../../../../source/slang/slang-ir-hlsl-legalize.cpp)) | `HLSLSourceEmitter` ([slang-emit-hlsl.cpp](../../../../source/slang/slang-emit-hlsl.cpp)) | DXC (for `DXIL*`), fxc (for `DXBytecode*`) | **No** loops in `linkAndOptimizeIR`. |
| Metal | `Metal`, `MetalLib`, `MetalLibAssembly` | `legalizeIRForMetal` ([slang-ir-metal-legalize.cpp](../../../../source/slang/slang-ir-metal-legalize.cpp)) | `MetalSourceEmitter` ([slang-emit-metal.cpp](../../../../source/slang/slang-emit-metal.cpp)) | Apple `metal` compiler (for `MetalLib*`) | **No** loops in `linkAndOptimizeIR`; `legalizeIRForMetal` is single-pass. |
| WGSL | `WGSL`, `WGSLSPIRV`, `WGSLSPIRVAssembly` | `legalizeIRForWGSL` ([slang-ir-wgsl-legalize.cpp](../../../../source/slang/slang-ir-wgsl-legalize.cpp)) | `WGSLSourceEmitter` ([slang-emit-wgsl.cpp](../../../../source/slang/slang-emit-wgsl.cpp)) | Tint (for `WGSLSPIRV*`) | **No** loops in `linkAndOptimizeIR`; `legalizeIRForWGSL` is single-pass. |
| CUDA | `CUDASource`, `CUDAHeader`, `PTX` | (no single entry; per-pass CUDA arms, e.g. `lowerImmutableBufferLoadForCUDA` in [slang-ir-cuda-immutable-load.cpp](../../../../source/slang/slang-ir-cuda-immutable-load.cpp)) | `CUDASourceEmitter` ([slang-emit-cuda.cpp](../../../../source/slang/slang-emit-cuda.cpp), inheriting from `CPPSourceEmitter`) | nvrtc / runtime CUDA compiler (for `PTX`) | **No** loops at the `linkAndOptimizeIR` level; one pass has an internal fixed-point loop, documented in [cuda.md](cuda.md). |

Two entries in that table need a caveat.

**"No single entry" versus a named driver.** Metal and WGSL each
have one `SLANG_PASS(legalizeIRFor*)` call inside
`linkAndOptimizeIR` that owns essentially all of their
target-specific legalization. HLSL, CUDA, and SPIR-V do not, but
for different reasons: HLSL and CUDA genuinely scatter their
target-specific work across several individual switch arms (the
passes named above are examples, not the full inventory — each
child page lists them all), whereas SPIR-V *does* have a
single driver — it simply runs later, inside the emit step, so it
belongs to Phase D on [spirv.md](spirv.md) rather than Phase C.

**The SPIR-V loop bounds are not enforced.** At
[slang-ir-spirv-legalize.cpp](../../../../source/slang/slang-ir-spirv-legalize.cpp)
lines 3130-3165, `simplifyIRForSpirvLegalization` declares
`kMaxIterations = 8` with `iterationCounter = 0` and
`kMaxFuncIterations = 16` with `funcIterationCount = 0`, but
neither counter is ever incremented. The loop conditions
`while (changed && iterationCounter < kMaxIterations)` and its
inner equivalent are therefore effectively `while (changed)`: both
loops run to convergence, and the 8 / 16 figures are a *nominal*
bound that the source declares but never applies — so neither loop
has an enforced worst-case iteration count.
[spirv.md](spirv.md) documents the consequences, and the outer
loop's additional error-count break, in its Loops section.

Beyond those, targets differ in which conditional gates and
target-specific legalization arms they land on; each per-target
page documents those choices, its phi-elimination configuration,
and its downstream tool chain in detail.

## Filtering rules

There are two independent reasons a pass may be absent from a
per-target page: the arm is gated on a *different target*, or the
pass is gated on the module *not containing* the IR it would
transform. The first is what makes the five pages differ from each
other; the second is what makes any one page differ from compile to
compile.

### Filtering by target

Each per-target page filters out switch arms gated on a sibling
target (`isSPIRV`, `isMetalTarget`, `isWGPUTarget`, `isCUDATarget`,
`isD3DTarget`, `isKhronosTarget`, `target == HLSL`,
`target == GLSL`, `target == CodeGenTarget::PyTorchCppBinding`,
the CPU / Host / LLVM variants, etc.). A glance at one page does
**not** show the global ordering of `linkAndOptimizeIR`; it shows
only the passes reachable for that target. Where two targets
share an arm (for example, Metal, CUDA, and the CPP targets all hit
the `undoParameterCopy` arm at line 2524), each page that lists the
pass also documents the shared arm in its prose.

### Filtering by IR content: `RequiredLoweringPassSet`

Most backend passes are additionally skipped when the linked module
contains no IR that needs them. The predicate is
`struct RequiredLoweringPassSet`, declared at line 52 of
[../../../../source/slang/slang-code-gen.h](../../../../source/slang/slang-code-gen.h)
and filled by `calcRequiredLoweringPassSet` (line 409 of
[../../../../source/slang/slang-emit.cpp](../../../../source/slang/slang-emit.cpp)),
which `linkAndOptimizeIR` runs twice, at lines 1085 and 1666. For
most gates the condition is therefore not a target predicate at
all; each per-target page's `## Conditional gates` section lists
the flags that apply to that target.

For a single, unfiltered view of every pass — independent of both
kinds of filtering — read
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) and the
source of `linkAndOptimizeIR` directly. For the cross-cutting
per-target option and capability *model* — including its
"Profiles versus explicit `-capability`" section, which settles how
a requested profile relates to explicit capability atoms — see
[../cross-cutting/targets.md](../cross-cutting/targets.md); that
page owns the model, and the pages here do not restate it.

## See also

- [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) —
  AST → IR lowering.
- [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) —
  unordered topical catalog of IR passes.
- [../pipeline/06-emit.md](../pipeline/06-emit.md) — backend
  emit overview.
- [../cross-cutting/targets.md](../cross-cutting/targets.md) —
  per-target options, capability sets, and target predicates.
- [../ir-reference/index.md](../ir-reference/index.md) —
  per-opcode catalog.

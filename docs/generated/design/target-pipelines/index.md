---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T17:27:07Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 88909e4def1133ca5cd3ccb36f17d01f8bcc633abff88b21acd9208e1a05d1f2
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Target Pipelines

This page is a navigation hub for the per-target pipeline pages
in `target-pipelines/`, written for compiler developers who need
to pick the right per-target page. Each peer page documents one target's
ordered IR-pass and downstream-tool sequence as a four-phase
control-flow-graph view of the shared orchestrator
`linkAndOptimizeIR` (line 970 of
[../../../../source/slang/slang-emit.cpp](../../../../source/slang/slang-emit.cpp)).
For an unordered, topical catalog of every IR pass — grouped by
category rather than by execution order — see
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md). For the
capability / profile _model_ that decides what a target is allowed
to emit, see [../cross-cutting/targets.md](../cross-cutting/targets.md);
the pages here document per-target pipeline _behavior_ only.

## Pages

- [spirv.md](spirv.md) — SPIR-V via the direct-emit path
  (`emitSPIRVForEntryPointsDirectly`), plus the spirv-link /
  spirv-val / spirv-opt downstream chain. The only page with
  iterative passes; also covers `-debug-info-include-source`,
  `-Xspirv-opt` passthrough, descriptor-heap `ConstantBuffer`
  access, and `OpSwitch` case literals at the selector width.
- [hlsl.md](hlsl.md) — HLSL source with DXC (DXIL) and fxc
  (DXBytecode) downstream. Covers the work-graph feature set (the
  `node` stage, work-graph record types, and named-constant
  emission such as `[NodeLaunch("broadcasting")]`) and the
  `precise` qualifier, which reaches emitted source only on the
  HLSL and GLSL emitters — they inherit
  `CLikeSourceEmitter::emitTempModifiers`, while the Metal, WGSL,
  and CPP / CUDA emitters each override it to drop the qualifier.
- [metal.md](metal.md) — Metal source with the Apple `metal`
  compiler downstream for `MetalLib` / `MetalLibAssembly`. Covers
  `printf` mapped onto the MSL 3.2 shader-logging facility (which
  makes the emitter require MSL 3.2 and enable Metal logging for
  the downstream compile), half / float literal
  suffixes (`1.0h` / `1.0f`), and coverage counters capped at
  32 bits.
- [wgsl.md](wgsl.md) — WGSL source with Tint (WGSL → SPIR-V)
  downstream for `WGSLSPIRV` / `WGSLSPIRVAssembly`. Covers the
  diagnostic for `precise` (WGSL has no such keyword), the
  bool → int cast emitted as `select(T(0), T(1), cond)`, and the
  `shouldEmitSwitchCaseTerminatingBreak()` policy override, which
  is deliberately independent of `supportsSwitchFallThrough()`.
- [cuda.md](cuda.md) — CUDA C++ source / header with nvrtc (PTX)
  downstream, plus an `## Adjacent targets` section that briefly
  cross-links PyTorch / OptiX / host-CPP paths. Covers the OptiX
  payload write-back path when an entry point terminates from a
  nested callee, and the autodiff gate — which is
  target-independent, and which the PyTorch / `slangpy` binding
  path cross-linked from that section also reaches.

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
tools`, because SPIR-V is the one target whose _legalization
  driver_ runs inside the emit step rather than inside
  `linkAndOptimizeIR` (see the comparison table below).

Every page's `## Conditional gates` section now opens with a
`### requiredLoweringPassSet.* flags` subsection; that grouping
reflects the content-based gating described under
[Filtering rules](#filtering-rules).

Reading any single per-target page yields the **filtered** view
of `linkAndOptimizeIR` — passes that fire only for sibling
targets are omitted from the diagrams and tables. The shared
orchestrator runs unconditionally for every target; what differs
is which switch arm each target lands in.

## Cross-target comparison

| Target | CodeGenTarget enum values                                                     | Phase C entry                                                                                                                                                                                                                                       | Phase D emitter                                                                                                                                                                                                | Downstream tools                           | Loops                                                                                                                                                                                |
| ------ | ----------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| SPIR-V | `SPIRV`, `SPIRVAssembly`                                                      | (no single entry in `linkAndOptimizeIR`; per-pass SPIR-V arms) — the `legalizeIRForSPIRV` driver ([slang-ir-spirv-legalize.cpp](../../../../source/slang/slang-ir-spirv-legalize.cpp) line 3347) runs in **Phase D**, called from `emitSPIRVFromIR` | `emitSPIRVForEntryPointsDirectly` ([slang-emit.cpp](../../../../source/slang/slang-emit.cpp) line 3500) → `emitSPIRVFromIR` ([slang-emit-spirv.cpp](../../../../source/slang/slang-emit-spirv.cpp) line 12092) | spirv-link, spirv-val, spirv-opt           | **Yes** — the only target with iterative passes: `simplifyIRForSpirvLegalization` and the forward-declared-pointer fixup in `emitSPIRVFromIR`, both to convergence (see note below). |
| HLSL   | `HLSL` (plus downstream `DXIL`, `DXBytecode`, and their `*Assembly` variants) | (no single entry; per-pass HLSL arms, e.g. `legalizeRayPayloadAccessQualifiersForHLSL` and `validateBarrierFlagsForHLSL` in [slang-ir-hlsl-legalize.cpp](../../../../source/slang/slang-ir-hlsl-legalize.cpp))                                      | `HLSLSourceEmitter` ([slang-emit-hlsl.cpp](../../../../source/slang/slang-emit-hlsl.cpp))                                                                                                                      | DXC (for `DXIL*`), fxc (for `DXBytecode*`) | **No** loops in `linkAndOptimizeIR`.                                                                                                                                                 |
| Metal  | `Metal`, `MetalLib`, `MetalLibAssembly`                                       | `legalizeIRForMetal` ([slang-ir-metal-legalize.cpp](../../../../source/slang/slang-ir-metal-legalize.cpp))                                                                                                                                          | `MetalSourceEmitter` ([slang-emit-metal.cpp](../../../../source/slang/slang-emit-metal.cpp))                                                                                                                   | Apple `metal` compiler (for `MetalLib*`)   | **No** loops in `linkAndOptimizeIR`; `legalizeIRForMetal` is single-pass.                                                                                                            |
| WGSL   | `WGSL`, `WGSLSPIRV`, `WGSLSPIRVAssembly`                                      | `legalizeIRForWGSL` ([slang-ir-wgsl-legalize.cpp](../../../../source/slang/slang-ir-wgsl-legalize.cpp))                                                                                                                                             | `WGSLSourceEmitter` ([slang-emit-wgsl.cpp](../../../../source/slang/slang-emit-wgsl.cpp))                                                                                                                      | Tint (for `WGSLSPIRV*`)                    | **No** loops in `linkAndOptimizeIR`; `legalizeIRForWGSL` is single-pass.                                                                                                             |
| CUDA   | `CUDASource`, `CUDAHeader`, `PTX`                                             | (no single entry; per-pass CUDA arms — `lowerImmutableBufferLoadForCUDA` in [slang-ir-cuda-immutable-load.cpp](../../../../source/slang/slang-ir-cuda-immutable-load.cpp) is the one pass that exists solely for this target family)                | `CUDASourceEmitter` ([slang-emit-cuda.cpp](../../../../source/slang/slang-emit-cuda.cpp), inheriting from `CPPSourceEmitter`)                                                                                  | nvrtc / runtime CUDA compiler (for `PTX`)  | **No** loops in `linkAndOptimizeIR`.                                                                                                                                                 |

Two entries in that table need a caveat.

**"No single entry" versus a named driver.** Metal and WGSL each
have one `SLANG_PASS(legalizeIRFor*)` call inside
`linkAndOptimizeIR` that owns essentially all of their
target-specific legalization. HLSL, CUDA, and SPIR-V do not, but
for different reasons: HLSL and CUDA genuinely scatter their
target-specific work across several individual switch arms (the
passes named above are examples, not the full inventory — each
child page lists them all), whereas SPIR-V _does_ have a
single driver — it simply runs later, inside the emit step, so it
belongs to Phase D on [spirv.md](spirv.md) rather than Phase C.

**The SPIR-V loop bounds are not enforced.** At
[slang-ir-spirv-legalize.cpp](../../../../source/slang/slang-ir-spirv-legalize.cpp)
lines 3124-3145, `simplifyIRForSpirvLegalization` declares
`kMaxIterations = 8` with `iterationCounter = 0` and
`kMaxFuncIterations = 16` with `funcIterationCount = 0`, but
neither counter is ever incremented. The loop conditions
`while (changed && iterationCounter < kMaxIterations)` and its
inner equivalent are therefore effectively `while (changed)`: both
loops run to convergence, and the 8 / 16 figures are a _nominal_
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
per-target page: the arm is gated on a _different target_, or the
pass is gated on the module _not containing_ the IR it would
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
the `undoParameterCopy` arm at line 2340), each page that lists the
pass also documents the shared arm in its prose.

### Filtering by IR content: `RequiredLoweringPassSet`

Most backend passes are additionally skipped when the linked module
contains no IR that needs them. The predicate is
`struct RequiredLoweringPassSet`, declared at line 52 of
[../../../../source/slang/slang-code-gen.h](../../../../source/slang/slang-code-gen.h)
— a record of 34 independent `bool` flags, one per lowering
concern (`enumType`, `taggedUnion`, `autodiff`,
`appendConsumeStructuredBuffer`, `reinterpret`, and so on). It is
filled by `calcRequiredLoweringPassSet` (line 405 of
[../../../../source/slang/slang-emit.cpp](../../../../source/slang/slang-emit.cpp)),
which walks the module and sets a flag for every construct it
finds. `linkAndOptimizeIR` runs that scan twice — at lines 1049 and
1520 — so constructs introduced by specialization can still turn a
gate on. Flags accumulate rather than reset, so the second scan can
only add.

This is why every per-target page's `## Conditional gates` table
now leads with a `### requiredLoweringPassSet.* flags` subsection:
for most rows the gate is not a target predicate at all. Two
properties of the mechanism cut across all five targets and are
easy to get wrong when reading a single page:

- **The autodiff gate is a branch, not a skip.** At lines
  1446-1453 of `slang-emit.cpp`, a false
  `requiredLoweringPassSet.autodiff` does not simply omit
  `finalizeAutoDiffPass` — the `else` arm runs
  `stripAutoDiffDecorations` instead. That arm is required rather
  than merely tidy: even a module with no autodiff constructs links
  in the core-module autodiff builtins, which carry
  `ExportDecoration` / `HLSLExportDecoration` /
  `KeepAliveDecoration` and are thereby pinned against dead-code
  elimination. Stripping those decorations is
  what lets the following `eliminateDeadCode` drop the unused
  builtins.
- **One flag is mutated mid-pipeline.** At line 1609,
  `lowerTaggedUnionTypes` sets
  `requiredLoweringPassSet.reinterpret = true` from inside its own
  `taggedUnion` gate, because lowering a tagged union produces new
  `reinterpret` instructions for `lowerReinterpret` a few lines
  later to consume. Every other flag is written only by the two
  scans, so this is the single place where the gate set is a
  function of a pass _result_ rather than of a module walk.

For a single, unfiltered view of every pass — independent of both
kinds of filtering — read
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) and the
source of `linkAndOptimizeIR` directly. For the cross-cutting
per-target option and capability _model_ — including its
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

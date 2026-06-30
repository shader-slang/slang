# Prompt: target-pipelines/index.md

See [_common.md](_common.md) for the universal rules and the
**Target-pipeline index contract**.

## Target

Produce `docs/generated/design/target-pipelines/index.md` — a
compact navigation hub for the per-target pipeline pages. This is
not a target page; do not document any pass.

Audience: a developer landing on the `target-pipelines/` subtree
who needs to pick the right per-target page and understand why all
the pages share the same four-phase shape.

## Sources

- [slang-emit.cpp](../../../../source/slang/slang-emit.cpp)
  `linkAndOptimizeIR` (line ~892) — the shared orchestrator that
  every per-target page describes a filtered view of.
- The five peer pages:
  [spirv.md](spirv.md), [hlsl.md](hlsl.md), [metal.md](metal.md),
  [wgsl.md](wgsl.md), [cuda.md](cuda.md).

## Page shape

Follow the **Target-pipeline index contract** in
[_common.md](_common.md):

1. `# Target Pipelines` title.
2. One-paragraph intro stating the purpose: each peer page is the
   ordered, CFG-style view of one target's pipeline. Point readers
   at [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)
   for the unordered topical catalog.
3. `## Pages` — bullet list, one entry per peer page, with the
   page link and a one-clause description.
4. `## Shared shape` — short paragraph explaining the four phases
   once (Phase A: link + entry-point prep; Phase B: specialization
   + type legalization; Phase C: target legalization, lowering,
   phi elimination; Phase D: emit + downstream tools). Reference
   the contract section in `_common.md`. Identify the shared
   orchestrator (`linkAndOptimizeIR`).
5. `## Cross-target comparison` — a single table with columns
   **Target**, **CodeGenTarget enum values**, **Phase C entry**,
   **Phase D emitter**, **Downstream tools**, **Loops**.
   Use these exact entry/emitter names:

   | Target | Enum values | Phase C entry | Phase D emitter | Downstream | Loops |
   | --- | --- | --- | --- | --- | --- |
   | SPIR-V | `SPIRV`, `SPIRVAssembly` | `legalizeIRForSPIRV` | `emitSPIRVForEntryPointsDirectly` | spirv-link, spirv-val, spirv-opt | `simplifyIRForSpirvLegalization` (outer 8 x inner 16); forward-declared-pointer fixup |
   | HLSL | `HLSL` (plus downstream `DXIL`, `DXBytecode`) | (no single entry; per-pass HLSL arms) | `HLSLSourceEmitter` | DXC, fxc | none in `linkAndOptimizeIR` |
   | Metal | `Metal`, `MetalLib`, `MetalLibAssembly` | `legalizeIRForMetal` | `MetalSourceEmitter` | Apple `metal` compiler (for `MetalLib*`) | none in `linkAndOptimizeIR` |
   | WGSL | `WGSL`, `WGSLSPIRV`, `WGSLSPIRVAssembly` | `legalizeIRForWGSL` | `WGSLSourceEmitter` | Tint (for `WGSLSPIRV*`) | none in `linkAndOptimizeIR` |
   | CUDA | `CUDASource`, `CUDAHeader`, `PTX` | (no single entry; per-pass CUDA arms) | `CUDASourceEmitter` | nvrtc (for `PTX`) | none in `linkAndOptimizeIR` |

6. `## Filtering rules` — a short paragraph reminding the reader
   that each target page filters out switch arms gated on a
   sibling target (e.g. `isCUDATarget`, `target == HLSL`), so a
   glance at one page is not the global ordering. Cross-link the
   shared orchestrator `linkAndOptimizeIR` for the unfiltered
   view.
7. `## See also` — bullets:
   - [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md)
   - [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)
   - [../pipeline/06-emit.md](../pipeline/06-emit.md)
   - [../cross-cutting/targets.md](../cross-cutting/targets.md)
   - [../ir-reference/index.md](../ir-reference/index.md)

## Quality checklist

- [ ] No per-pass details copied from any target page.
- [ ] Every peer page in `target-pipelines/` (other than this
      file) appears in both the `## Pages` bullet list and the
      cross-target table.
- [ ] The cross-target table columns are in the exact order
      specified.
- [ ] Under the size cap (32 KB).

---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ir-reference/decorations.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 11
actions:
  fixed: 8
  rejected_bogus: 2
  rejected_out_of_scope: 1
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/decorations.md

## Summary

This is a re-run triggered by eight gaps recorded as `fixed` that the
reporting bundle still reports. No gap is a compiler defect, so none is
escalated. Every earlier fix was re-checked against the source at
`2518a942d`, which also moved nearly every cited line number; those
citations were corrected during the refresh. The eight reopened gaps
stay `fixed`: for the ones whose gap anchors at a table row but whose
fix had lived in a separate callout, the fact is now stated in the row
itself, and the `missing-example` gaps now have a worked example. The
three gaps not reopened (`a13d8a468a77`, `7bb8913f2a18`,
`69f3396b03eb`) carry their earlier verdicts forward. Breakdown: eight
fixed, two rejected as bogus, one rejected as out of scope. The
compiler binary in this tree is a Linux build and could not run, so
every example below is derived from source, not from a dump.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| a13d8a468a77 | rejected-out-of-scope | `docs/generated/design/_meta/prompts/ir-reference-decorations.md` `## Forbidden content`: backend-specific consumption of decorations belongs to `docs/generated/design/pipeline/06-emit.md`, which the callout already links. Carried forward unchanged. | — |
| 33fbeb8b7617 | fixed | `source/slang/slang-lower-to-ir.cpp:16563-16577` inside `TargetProgram::createIRModuleForLayout` (line 16434): the decoration is added from `entryPointLayout->inferredCapabilityRequirements`, only for atoms in `_spirv_1_0..latestSpirvAtom` or `metallib_2_3..latestMetalAtom`; builder at `source/slang/slang-ir-insts.h:5058`. `spirv_1_5` is a public alias at `source/slang/slang-capabilities.capdef:1662`. No `[require]` path adds it directly. | row now states the layout-IR origin plus an inline `[require(spirv_1_5)]` pointer, and a new `requireCapabilityAtom` callout gives the worked entry-point example |
| 320bf7714a3d | fixed | `source/slang/slang-ir-insts.h:382` (`getProfile()` rebuilds a `Profile` from `Profile::RawVal`), `source/slang/slang-ir-insts.h:5278-5288` (`addEntryPointDecoration` stores `profile.raw`), `source/slang/slang-profile.h:50-58,75-76,96-101` (`Stage` low 16 bits, `ProfileVersion` high 16). Stage integers are `include/slang.h:907-923` (`SlangStage`), bound by `PROFILE_STAGE` entries. | `entryPoint` callout now carries the tag-to-stage table (1 vertex through 16 node) and the `(version << 16) \| stage` note |
| dbe97ebb7283 | fixed | `source/slang/core.meta.slang:4867` and `4874` declare `attribute_syntax [format(format : String)]` / `[vk_image_format(format : String)] : FormatAttribute`; `source/slang/slang-lower-to-ir.cpp:3229-3231` passes `formatAttr->format` to `addFormatDecoration`; `source/slang/slang-ir-insts.h:5349-5351` stores `getIntValue(getIntType(), IRIntegerValue(format))`. | `format` row shows the quoted argument, says the operand is the integer `ImageFormat`, and adds a one-line `RWTexture2D` example; the integer for each name is not given because the format table is outside the watched paths |
| e19c29a6b653 | fixed | `source/slang/slang-ir-insts.h:181-190` (`enum class IRInterpolationMode`: Linear, NoPerspective, NoInterpolation, Centroid, Sample, PerVertex) and `source/slang/slang-lower-to-ir.cpp:3165-3187` (one modifier to one value). The gap's list omitted `pervertex` 5. | the mapping now sits in the `interpolationMode` row itself (the gap anchors at the table), not only in the callout |
| 7bb8913f2a18 | rejected-bogus | `source/slang/core.meta.slang:4508` declares `attribute_syntax [vk_location(location : int)] : GLSLLocationAttribute`, so `vk_location` is a real writable spelling, and `vk::location` reaches it through the scoped-name rewrite in `source/slang/slang-parser.cpp`. The row already leads with `[vk::location(N)]`. Carried forward unchanged. | — |
| 863b0c0f87dc | fixed | `source/slang/core.meta.slang:4559-4560` (`__attributeTarget(LoopStmt)`, `[ForceUnroll(count: int = 0)]`), `source/slang/slang-lower-to-ir.cpp:7973-7975` (read off the loop statement), `source/slang/slang-ir-insts.h:4911-4914` (builder appends the integer) versus `source/slang/slang-ir-insts.lua:2226-2229` (no declared operand). The `ForceUnroll` row already states all three. | verified the existing row (operand `count`, default `0`, loop-statement-only target) against current lines; no new text was needed |
| 69f3396b03eb | rejected-bogus | Two distinct opcodes: `source/slang/slang-ir-insts.lua:2108` (`streamOutputTypeDecoration`, a decoration) and `source/slang/slang-ir-insts.lua:2925` (`streamOutputTypeLayout`, under `TypeLayout`, wrapped by `IRStreamOutputTypeLayout` at `source/slang/slang-ir-insts.h:1433`). The decoration is produced only at `source/slang/slang-ir-glsl-legalize.cpp:4182`. Carried forward; the row now also names the layout opcode so a name search is not misleading. | — |
| bd7183d2d708 | fixed | `source/slang/slang-lower-to-ir.cpp:12423-12425` (only caller, in `visitInterfaceDecl`, line 12132, under `findModifier<BuiltinAttribute>()`); `source/slang/slang-ir-insts.h:5391` (only builder). Every `[builtin]` use in `source/slang/core.meta.slang` (13) and `source/slang/hlsl.meta.slang` (6) precedes an `interface`; `IBufferDataLayout` is `source/slang/hlsl.meta.slang:24-26` and is the default bound at line 76. | row now says it never appears on user declarations and to anchor searches at a user symbol; new `BuiltinDecoration` callout gives the `RWStructuredBuffer` example |
| 93335aeba9c7 | fixed | `source/slang/slang-ir-insts.h:927` (`getSynthesizedStatus()` reads an `IRBoolLit`), `source/slang/slang-ir-insts.h:4928-4931` (`addConstructorDecoration(IRInst*, bool)`), sole caller `source/slang/slang-lower-to-ir.cpp:14361-14364` passing `ConstructorFlavor::SynthesizedDefault`. | verified the existing `constructor` row (`true` means compiler-synthesized default) against current lines; no new text was needed |
| 901ca639c751 | fixed | Producers `source/slang/slang-ir-entry-point-uniforms.cpp:550` and `source/slang/slang-ir-collect-global-uniforms.cpp:157`; global collection is skipped unless ordinary data exists, `source/slang/slang-ir-collect-global-uniforms.cpp:120-122`. Both passes run in target lowering, so neither struct exists in the lowering-time module. The example in the callout was re-derived from these lines, not run. | callout kept and now cites both producer lines; the example (entry-point `uniform float scale`, then the same value as a global) is unchanged |

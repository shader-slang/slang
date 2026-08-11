---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:23:56Z
target_doc: ir-reference/decorations.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 11
actions:
  fixed: 7
  rejected_bogus: 2
  rejected_out_of_scope: 1
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/decorations.md

## Summary

Eleven gaps, all reported by `design/ir-reference/decorations`. Seven were
confirmed in the watched paths and fixed: the `ForceUnroll` operand and
loop-statement-only placement, the `requireCapabilityAtom` producer, the
`entryPoint` profile-tag encoding, the `interpolationMode` value mapping, the
quoted-string `format` argument, the `constructor` boolean operand, and the
real origin of `BuiltinDecoration`. Two were rejected as bogus after reading
the source: `streamOutputTypeLayout` is a separate `TypeLayout` opcode rather
than a second spelling of `streamOutputTypeDecoration`, and `[vk_location(N)]`
*is* a writable spelling (the parser folds `vk::location` onto it) — though
that row was still improved to lead with the spelling users actually write.
One is out of scope (per-target emit behaviour is forbidden content for this
page) and one is deferred because it can only be settled by running the
compiler. No gap turned out to be a compiler defect, so nothing was escalated.

Two `Suggested addition` hypotheses were wrong in a way that mattered and were
not written as proposed. `320bf7714a3d` proposed a stage-to-tag table; the tag
is a `Profile::RawVal` whose low 16 bits happen to be the stage, so a
stage-only table would have been a documented half-truth. `bd7183d2d708`
asserted `BuiltinDecoration` sits on "nearly every core-module declaration";
the sole producer attaches it only to `interface` declarations carrying
`[builtin]`, of which there are 18 across the two `.meta.slang` files.

## Manifest follow-up for the operator

Two claims this page now makes, and one it should make, rest on files outside
`watched_paths`:

- `source/slang/slang-profile.h` — owns the `Profile::RawVal` layout
  (`(ProfileVersion << 16) | Stage`) that the new `entryPoint` callout cites.
  The page cannot detect drift in that encoding today.
- `source/slang/slang-ir-glsl-legalize.cpp` — is the *second* producer of
  `requireCapabilityAtom` (line 5077, on the module inst) and the *only*
  producer of `streamOutputTypeDecoration` (line 4076). The page already cites
  this file in four rows without watching it. Adding it would let the
  `streamOutputTypeDecoration` row name its real producer instead of the
  current "Geometry shader output declaration", which is the underlying reason
  a reader searching an HLSL dump finds nothing.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| a13d8a468a77 | rejected-out-of-scope | `docs/generated/design/_meta/prompts/ir-reference-decorations.md` lines 75-81, `## Forbidden content`: "Backend-specific consumption of decorations — see [../pipeline/06-emit.md]". A per-target table of what `[unroll]` / `[branch]` become in hlsl / glsl / spirv / metal / wgsl is exactly that material, and the callout already links `06-emit.md`. | — |
| 33fbeb8b7617 | fixed | `source/slang/slang-lower-to-ir.cpp:16509-16521`, inside `TargetProgram::createIRModuleForLayout` (line 16381): the decoration is built from `asFuncDecl->inferredCapabilityRequirements`, filtered to `_spirv_1_0..latestSpirvAtom` and `metallib_2_3..latestMetalAtom`. No `[require(...)]` path adds it. Builder at `source/slang/slang-ir-insts.h:5014-5019`. | replaced the false `require(...)` AST origin with the layout-IR producer and the inferred-capability-set route, and noted that only SPIR-V-version and Metal-library atoms are recorded |
| 320bf7714a3d | fixed | `source/slang/slang-ir-insts.h:355` — `getProfile()` returns `Profile(Profile::RawVal(getIntVal(getProfileInst())))`; `source/slang/slang-ir-insts.h:5228-5238` stores `profile.raw`; `source/slang/slang-lower-to-ir.cpp:15274-15278` passes `entryPoint->getProfile()`. Encoding at `source/slang/slang-profile.h:75-77,96-101`. Value `1` for `-stage vertex` verified by `entry-point-vertex-profile.slang`. | extended the `entryPoint` callout to say the tag is a `Profile::RawVal` (`ProfileVersion` high 16 bits, `Stage` low 16), not a bare stage code |
| dbe97ebb7283 | fixed | `source/slang/core.meta.slang:4749-4750` and `4756-4757` — `attribute_syntax [format(format : String)]` / `[vk_image_format(format : String)] : FormatAttribute`; `source/slang/slang-ir-insts.h:5299-5302` — `addFormatDecoration(IRInst*, ImageFormat)` stores `getIntValue(getIntType(), IRIntegerValue(format))`. | showed the quoted argument in the `format` row and said the operand is the integer `ImageFormat` value the string resolves to |
| e19c29a6b653 | fixed | `source/slang/slang-ir-insts.h:153-164` — `enum class IRInterpolationMode { Linear, NoPerspective, NoInterpolation, Centroid, Sample, PerVertex }`; modifier mapping at `source/slang/slang-lower-to-ir.cpp:3125-3149`; source spellings at `source/slang/slang-parser.cpp:10792-10796` and `source/slang/core.meta.slang:42`. Values 0/2/4 pinned by `interpolation-mode-{linear,nointerpolation,sample}.slang`. | added an `interpolationMode` callout with the full six-value mapping (the gap's list omitted `PerVertex` 5) and pointed the table row at it |
| 7bb8913f2a18 | rejected-bogus | `source/slang/core.meta.slang:4418-4419` declares `attribute_syntax [vk_location(location : int)] : GLSLLocationAttribute`, so `vk_location` is the attribute's real name and is writable; `source/slang/slang-parser.cpp:963-978` (`parseAttributeName`) rewrites each `::` in a scoped attribute name to `_`, which is how `[vk::location(7)]` in `glsl-location-decoration.slang` reaches the same attribute. The gap's premise — that `[vk_location(...)]` is unwritable — is false. The row was still reworded to lead with the spelling users write. | — |
| 863b0c0f87dc | fixed | `source/slang/core.meta.slang:4469-4471` — `__attributeTarget(LoopStmt)` + `attribute_syntax [ForceUnroll(count: int = 0)]`; `source/slang/slang-lower-to-ir.cpp:8409-8411` reads the modifier off the loop *statement*; `source/slang/slang-ir-insts.h:4867-4870` — `addLoopForceUnrollDecoration` appends `getIntValue(getIntType(), iters)` even though the Lua entry (`source/slang/slang-ir-insts.lua:2199-2202`) declares none. Operand form pinned by `force-unroll-on-loop.slang`, E31002 by `force-unroll-on-function-error.slang`. | corrected the `ForceUnroll` row to the builder-appended `count: IRIntLit` idiom the `vulkanRayPayload` rows already use, and recorded the `LoopStmt`-only attribute target and the `0` default |
| 69f3396b03eb | rejected-bogus | These are two distinct opcodes, not two spellings of one. `source/slang/slang-ir-insts.lua:2081-2084` declares the decoration `streamOutputTypeDecoration`; `source/slang/slang-ir-insts.lua:2888` declares `streamOutputTypeLayout` under `TypeLayout`, wrapped by `IRStreamOutputTypeLayout : IRTypeLayout` at `source/slang/slang-ir-insts.h:1406-1424`. What `stream-output-type-decoration.slang` matched is the type-layout inst. The decoration itself is produced only by `source/slang/slang-ir-glsl-legalize.cpp:4076`, so an `-target hlsl` dump cannot contain it. | — |
| bd7183d2d708 | fixed | `source/slang/slang-ir-insts.h:5341` is the only definition of `addBuiltinDecoration` and `source/slang/slang-lower-to-ir.cpp:12322-12325` its only caller, inside `visitInterfaceDecl` (line 12073) under `decl->findModifier<BuiltinAttribute>()`. `[builtin]` is declared at `source/slang/core.meta.slang:4874-4875`; all 12 uses in `core.meta.slang` and all 6 in `hlsl.meta.slang` are on `interface` declarations, including `IBufferDataLayout` at `source/slang/hlsl.meta.slang:22-24`, which is the default layout constraint of `RWStructuredBuffer` (`hlsl.meta.slang:74`). | narrowed the row from "Core-module lowering" / "an inst" to the `[builtin]` attribute on an `interface`, and named `IBufferDataLayout` as the case ordinary code links in |
| 93335aeba9c7 | fixed | `source/slang/slang-ir-insts.h:895-901` — `IRConstructorDecoration::getSynthesizedStatus()` reads `cast<IRBoolLit>(getOperand(0))`; `source/slang/slang-ir-insts.h:4884-4887` — `addConstructorDecoration(IRInst*, bool synthesizedConstructor)`; sole caller `source/slang/slang-lower-to-ir.cpp:14302-14305` passes `constructorDecl->containsFlavor(ConstructorDecl::ConstructorFlavor::SynthesizedDefault)`. | replaced the retired `(variadic, min=1)` cell with `(1 unnamed: an `IRBoolLit`, read by `getSynthesizedStatus()`)` and said `true` marks the compiler-synthesized default rather than a user-written `__init` |
| 901ca639c751 | deferred | Both producers are outside `watched_paths` — `source/slang/slang-ir-entry-point-uniforms.cpp:550` and `source/slang/slang-ir-collect-global-uniforms.cpp:157` — and the gap asks for a program plus target/options that demonstrably *reaches* one of them. Deciding which entry-point-`uniform` or global-`uniform` shape actually triggers collection requires running `slangc -dump-ir`, which this host cannot do (the tree's build is Linux x86-64, host is arm64). Follow-up: add both files to `watched_paths` and settle the reachable case on a machine that can run the compiler. | — |

---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:02:55+00:00
source_commit: ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
watched_paths_digest: 4b3db073637a83dee640e8a3acff6c04c7a14dfd74b4925331059caecfdf8799
source_doc: docs/llm-generated/pipeline/04c-layout-ir.md
source_doc_digest: 6691cbbba6704911ce5609e086a2bc96ec301961cf6fc5c9f5ae8be9ab0e372c
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/04c-layout-ir

## Intent

Tests verify the per-target layout IR module construction described
in
[`docs/llm-generated/pipeline/04c-layout-ir.md`](../../../docs/llm-generated/pipeline/04c-layout-ir.md):
`TargetProgram::createIRModuleForLayout` builds a sibling IR module
to the executable IR, carrying `IRLayoutDecoration`s on stub globals
and entry-point functions for one specific target's chosen layout
rules.

The layout IR module is **not** exposed under its own `### LAYOUT-IR:`
header in `-dump-ir` output. Its effects are visible in the first
post-link snapshot — `### AFTER validateAndRemoveAssumeAddress:` —
which contains the layout decorations and `EntryPointLayout` /
`structTypeLayout` / `varLayout` / `parameterGroupTypeLayout`
instructions merged from the layout-IR module. The pre-link
`### LOWER-TO-IR:` block is layout-free, which is the doc's "What
this module is not" claim.

All tests compile with
`//TEST:SIMPLE(filecheck=CHECK):-target <T> -dump-ir -o /dev/null
-entry main -stage compute` for `T` in `{spirv-asm, hlsl}` and
FileCheck the appropriate dump section. The bundle covers the
per-global-parameter loop, the global-scope type layout (both the
plain-struct and parameter-group paths), the per-entry-point loop
(layout decoration, EntryPointLayout shape, stage/semantic operands,
mangled-name linkage), the executable-IR-is-layout-free claim, and
the per-target observability of layout. Several gates (capability
filter, obfuscation, lazy cache) are recorded under `## Untested claims`.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| After the per-global-parameter loop, _lowerTypeLayoutCommon builds an IRStructTypeLayout for the global scope; with no cbuffer wrapping, the merged IR shows a top-level structTypeLayout(...) for the global struct (no parameterGroupTypeLayout). | functional | [#global-scope-type-layout](../../../docs/llm-generated/pipeline/04c-layout-ir.md#global-scope-type-layout) | [`global-scope-struct-type-layout-built.slang`](global-scope-struct-type-layout-built.slang) |
| When the global scope is wrapped in a parameter group (cbuffer), the module's layout decoration is an IRParameterGroupTypeLayout rather than the raw IRStructTypeLayout; observable as parameterGroupTypeLayout(...) in the layout block. | functional | [#global-scope-type-layout](../../../docs/llm-generated/pipeline/04c-layout-ir.md#global-scope-type-layout) | [`cbuffer-produces-parameter-group-type-layout.slang`](cbuffer-produces-parameter-group-type-layout.slang) |
| createIRModuleForLayout calls addLayoutDecoration on the module instance with irGlobalScopeVarLayout; the merged IR shows a top-level `varLayout(...)` instruction (not bound to `let`). | functional | [#global-scope-type-layout](../../../docs/llm-generated/pipeline/04c-layout-ir.md#global-scope-type-layout) | [`module-instance-has-toplevel-varlayout.slang`](module-instance-has-toplevel-varlayout.slang) |
| Per-entry-point step 5 attaches an addImportDecoration to the stub IRFunc; after link, the entry-point func resolves to its executable-side body and carries an [export("...mangled...")] decoration with the same mangled name. | functional | [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps) | [`entry-point-func-keeps-export-decoration.slang`](entry-point-func-keeps-export-decoration.slang) |
| Per-entry-point step 8 attaches an IRLayoutDecoration to the stub IRFunc; the merged entry-point func carries a [layout(%N)] decoration that the reflection API queries. | functional | [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps) | [`entry-point-func-gets-layout-decoration.slang`](entry-point-func-gets-layout-decoration.slang) |
| lowerEntryPointLayout encodes parameter bindings and stage into the IREntryPointLayout; the merged IR shows a stage(...) operand inside the entry-point's varLayout and a systemValueSemantic(...) operand for SV_DispatchThreadID. | functional | [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps) | [`entry-point-layout-encodes-stage-and-semantic.slang`](entry-point-layout-encodes-stage-and-semantic.slang) |
| lowerEntryPointLayout produces an IREntryPointLayout encoding parameter bindings and stage; the EntryPointLayout(...) instruction in the merged IR takes two varLayout(...) operands (parameters and result). | functional | [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps) | [`entry-point-layout-has-param-and-result-operands.slang`](entry-point-layout-has-param-and-result-operands.slang) |
| Each per-global-parameter step in createIRModuleForLayout attaches an IRLayoutDecoration to the stub IRGlobalVar; observable as a [layout(%N)] on the global_param. | functional | [#per-global-parameter-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-global-parameter-steps) | [`global-param-gets-layout-decoration.slang`](global-param-gets-layout-decoration.slang) |
| The per-global-parameter loop runs for each varLayout in globalStructLayout.fields; with two globals, the IRStructTypeLayout records two structFieldLayout entries and both stubs carry [layout(...)] decorations. | functional | [#per-global-parameter-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-global-parameter-steps) | [`multiple-globals-each-get-layout.slang`](multiple-globals-each-get-layout.slang) |
| globalStructTypeLayoutBuilder.addField(irVar, irLayout) feeds the IRStructTypeLayout; each global parameter becomes a structFieldLayout(field, varLayout) entry in the merged layout block. | functional | [#per-global-parameter-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-global-parameter-steps) | [`per-global-struct-field-layout-entries.slang`](per-global-struct-field-layout-entries.slang) |
| lowerVarLayout produces an IRVarLayout encoding binding/offset/space; the merged layout block contains a varLayout(...) with an offset(...) operand. | functional | [#per-global-parameter-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-global-parameter-steps) | [`global-param-varlayout-has-offset-operand.slang`](global-param-varlayout-has-offset-operand.slang) |
| The pre-link executable IR (### LOWER-TO-IR:) carries no [layout(...)] decorations; layout lives in a sibling module built by createIRModuleForLayout. | functional | [#what-this-module-is-not](../../../docs/llm-generated/pipeline/04c-layout-ir.md#what-this-module-is-not) | [`lower-to-ir-block-has-no-layout-decorations.slang`](lower-to-ir-block-has-no-layout-decorations.slang) |
| Layout is target-specific; the same global parameter produces a [layout(...)] / varLayout / offset structure on both SPIR-V and HLSL targets (this test asserts presence on HLSL to complement the default SPIR-V test). | functional | [#why-this-module-exists](../../../docs/llm-generated/pipeline/04c-layout-ir.md#why-this-module-exists) | [`layout-is-per-target-spirv-vs-hlsl.slang`](layout-is-per-target-spirv-vs-hlsl.slang) |

## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#irrequirecapabilityatomdecoration](../../../docs/llm-generated/pipeline/04c-layout-ir.md#irrequirecapabilityatomdecoration) | undocumented-behavior | The doc states the capability-atom filter at lines 15462-15463 emits `IRRequireCapabilityAtomDecoration`s only for SPIR-V and Metal layout-module entry points, but does not name a textual surface (e.g. a `requireCapabilityAtom(...)` instruction or `[requireCapabilityAtom(...)]` decoration name) by which a reader could observe the decoration in `-dump-ir` output. | Without that, comparing the post-link IR across SPIR-V vs HLSL targets is brittle (capability machinery is heavily rewritten by post-link passes). A doc sentence naming the IR opcode would unblock a focused test. |
| [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps) | undocumented-behavior | The doc's "Per-entry-point steps" table row 6 mentions forwarding capability atoms in the range `[_spirv_1_0, latestSpirvAtom]` or `[metallib_2_3, latestMetalAtom]`, but the precise atom values and their string mangling in `-dump-ir` output is not given. Treat as a doc gap pending an example. |  |
| [#optional-obfuscation-pass](../../../docs/llm-generated/pipeline/04c-layout-ir.md#optional-obfuscation-pass) | undocumented-behavior | The doc's "Optional obfuscation pass" describes the strip + DCE block but does not provide a user-observable consequence under `-obfuscate-code`. The doc could name the `IRNameHintDecoration` removal as the observable so a focused test could compare with/without `-obfuscate-code` flags. |  |
| [#cache-and-reuse](../../../docs/llm-generated/pipeline/04c-layout-ir.md#cache-and-reuse) | undocumented-behavior | The doc's "Cache and reuse" section describes lazy construction (`getOrCreateIRModuleForLayout`) and the `m_irModuleForLayout` cache field, but cache hit/miss is not user-observable through `slangc` (the only signal is correctness across multiple compilations). No actionable doc gap, but worth noting that cache semantics are inherently untestable from this bundle. |  |
| [#caveats-and-gotchas](../../../docs/llm-generated/pipeline/04c-layout-ir.md#caveats-and-gotchas) | undocumented-behavior | The doc's "Caveats and gotchas" lists `materialize` failure as `SLANG_UNEXPECTED("unhandled value flavor")` and the `SLANG_ASSERT(m_layout)` precondition; neither is reachable from user Slang source, so they cannot be tested. The doc could clarify that these are internal contract checks, not user-visible diagnostics. |  |

## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| **Optional obfuscation pass**: gated on `-obfuscate-code`, observable only via obfuscated source-map machinery outside this bundle's scope. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| **`SLANG_ASSERT(m_layout)` precondition**: a hard assertion for callers that bypass the lazy accessor; not user-reachable. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| **`buildMangledNameToGlobalInstMap`** call at the end of `createIRModuleForLayout`: no user-observable consequence (mirrors the same out-of-scope finding in 04b). | (unclassified) | [#buildmanglednametoglobalinstmap](../../../docs/llm-generated/pipeline/04c-layout-ir.md#buildmanglednametoglobalinstmap) | Reason and explanation to be refined by the next regeneration. |
| **No mandatory optimization passes run on the layout module**: the doc's "What this module is not" point that `constructSSA` / SCCP / CFG-simplify / DCE / early-inlining do not run on the layout side. The layout module's per-function bodies are stubs, so there is no positive textual signal; the negative signal (no SSA cleanup observable on a side that has no code) is unfalsifiable. | (unclassified) | [#constructssa](../../../docs/llm-generated/pipeline/04c-layout-ir.md#constructssa) | Reason and explanation to be refined by the next regeneration. |
| **`materialize` failure** (`SLANG_UNEXPECTED("unhandled value flavor")`): a hard crash for corrupted per-module IR caches; not user-reachable from Slang source. | (unclassified) | [#materialize](../../../docs/llm-generated/pipeline/04c-layout-ir.md#materialize) | Reason and explanation to be refined by the next regeneration. |
| **Lazy construction / cache reuse** of `m_irModuleForLayout`: no user-visible signal through `slangc`. | (unclassified) | [#mirmoduleforlayout](../../../docs/llm-generated/pipeline/04c-layout-ir.md#mirmoduleforlayout) | Reason and explanation to be refined by the next regeneration. |
| **Per-`TargetProgram` independence** (two layout modules in memory for two targets in one session): the cross-target shape difference is covered indirectly by `layout-is-per-target-spirv-vs-hlsl.slang` running on a separate invocation; in-process multi-target independence is not exposed through `slangc`. | (unclassified) | [#targetprogram](../../../docs/llm-generated/pipeline/04c-layout-ir.md#targetprogram) | Reason and explanation to be refined by the next regeneration. |
| **`IRRequireCapabilityAtomDecoration` filter** for SPIR-V / Metal only: see doc gaps above; the post-link pipeline rewrites capability metadata heavily, so no clean surface signal exists in `-dump-ir` to distinguish layout-module-attached from executable-module-attached capability atoms. | link-stage-only | [#irrequirecapabilityatomdecoration](../../../docs/llm-generated/pipeline/04c-layout-ir.md#irrequirecapabilityatomdecoration) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |

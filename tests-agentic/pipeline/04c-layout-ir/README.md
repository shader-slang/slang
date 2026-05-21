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
filter, obfuscation, lazy cache) are recorded under `## Out of scope`.

## Claims enumerated

| Claim ID | Anchor                                                                                                                       | Claim (one line)                                                                                                                                            | Tests                                                          |
| -------- | ---------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| C-01     | [#what-this-module-is-not](../../../docs/llm-generated/pipeline/04c-layout-ir.md#what-this-module-is-not)                    | The pre-link executable IR (`### LOWER-TO-IR:`) contains no `[layout(...)]` decorations; layout lives in a sibling module.                                  | [`lower-to-ir-block-has-no-layout-decorations.slang`](lower-to-ir-block-has-no-layout-decorations.slang)            |
| C-02     | [#per-global-parameter-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-global-parameter-steps)              | Each per-global step attaches an `IRLayoutDecoration` to the stub `IRGlobalVar`; the merged stub `global_param` carries `[layout(%N)]`.                     | [`global-param-gets-layout-decoration.slang`](global-param-gets-layout-decoration.slang)                    |
| C-03     | [#per-global-parameter-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-global-parameter-steps)              | `lowerVarLayout` produces an `IRVarLayout` encoding binding/offset; merged layout block contains `varLayout(...)` with an `offset(...)` operand.            | [`global-param-varlayout-has-offset-operand.slang`](global-param-varlayout-has-offset-operand.slang)              |
| C-04     | [#per-global-parameter-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-global-parameter-steps)              | The per-global loop runs for each `varLayout` in `globalStructLayout.fields`; two globals produce two `structFieldLayout(...)` entries.                     | [`multiple-globals-each-get-layout.slang`](multiple-globals-each-get-layout.slang)                       |
| C-05     | [#per-global-parameter-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-global-parameter-steps)              | `globalStructTypeLayoutBuilder.addField(irVar, irLayout)` feeds the `IRStructTypeLayout`; each global is a `structFieldLayout(field, varLayout)` entry.     | [`per-global-struct-field-layout-entries.slang`](per-global-struct-field-layout-entries.slang)                 |
| C-06     | [#global-scope-type-layout](../../../docs/llm-generated/pipeline/04c-layout-ir.md#global-scope-type-layout)                  | `addLayoutDecoration(irModule->getModuleInst(), irGlobalScopeVarLayout)` puts a top-level `varLayout(...)` line on the module instance.                     | [`module-instance-has-toplevel-varlayout.slang`](module-instance-has-toplevel-varlayout.slang)                 |
| C-07     | [#global-scope-type-layout](../../../docs/llm-generated/pipeline/04c-layout-ir.md#global-scope-type-layout)                  | Without a parameter-group wrapper, `_lowerTypeLayoutCommon` produces a plain `structTypeLayout(...)` for the global scope.                                  | [`global-scope-struct-type-layout-built.slang`](global-scope-struct-type-layout-built.slang)                  |
| C-08     | [#global-scope-type-layout](../../../docs/llm-generated/pipeline/04c-layout-ir.md#global-scope-type-layout)                  | When the global scope is wrapped in a `cbuffer`, the module's layout decoration is an `IRParameterGroupTypeLayout` — `parameterGroupTypeLayout(...)`.       | [`cbuffer-produces-parameter-group-type-layout.slang`](cbuffer-produces-parameter-group-type-layout.slang)           |
| C-09     | [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps)                        | Step 8 attaches an `IRLayoutDecoration` to the stub `IRFunc`; merged `func %main` carries `[layout(%N)]` queryable by reflection.                           | [`entry-point-func-gets-layout-decoration.slang`](entry-point-func-gets-layout-decoration.slang)                |
| C-10     | [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps)                        | `lowerEntryPointLayout` produces an `IREntryPointLayout`; the merged `EntryPointLayout(...)` takes two `varLayout(...)` operands (params, result).          | [`entry-point-layout-has-param-and-result-operands.slang`](entry-point-layout-has-param-and-result-operands.slang)       |
| C-11     | [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps)                        | `lowerEntryPointLayout` encodes stage / parameter bindings; merged IR shows `stage(...)` and `systemValueSemantic(...)` operands.                           | [`entry-point-layout-encodes-stage-and-semantic.slang`](entry-point-layout-encodes-stage-and-semantic.slang)          |
| C-12     | [#per-entry-point-steps](../../../docs/llm-generated/pipeline/04c-layout-ir.md#per-entry-point-steps)                        | Step 5 attaches `addImportDecoration(irFunc, mangledName)` to the stub; after link, the merged entry-point function carries an `[export("...main...")]`.    | [`entry-point-func-keeps-export-decoration.slang`](entry-point-func-keeps-export-decoration.slang)               |
| C-13     | [#why-this-module-exists](../../../docs/llm-generated/pipeline/04c-layout-ir.md#why-this-module-exists)                      | Layout is target-specific; the same source program yields layout instructions on HLSL as well as SPIR-V, confirming a per-target layout module is built.    | [`layout-is-per-target-spirv-vs-hlsl.slang`](layout-is-per-target-spirv-vs-hlsl.slang)                     |

## Tests in this bundle

| File                                                            | Intent     | Doc anchor                       |
| --------------------------------------------------------------- | ---------- | -------------------------------- |
| [`lower-to-ir-block-has-no-layout-decorations.slang`](lower-to-ir-block-has-no-layout-decorations.slang)             | functional | `#what-this-module-is-not`       |
| [`global-param-gets-layout-decoration.slang`](global-param-gets-layout-decoration.slang)                     | functional | `#per-global-parameter-steps`    |
| [`global-param-varlayout-has-offset-operand.slang`](global-param-varlayout-has-offset-operand.slang)               | functional | `#per-global-parameter-steps`    |
| [`multiple-globals-each-get-layout.slang`](multiple-globals-each-get-layout.slang)                        | functional | `#per-global-parameter-steps`    |
| [`per-global-struct-field-layout-entries.slang`](per-global-struct-field-layout-entries.slang)                  | functional | `#per-global-parameter-steps`    |
| [`module-instance-has-toplevel-varlayout.slang`](module-instance-has-toplevel-varlayout.slang)                  | functional | `#global-scope-type-layout`      |
| [`global-scope-struct-type-layout-built.slang`](global-scope-struct-type-layout-built.slang)                   | functional | `#global-scope-type-layout`      |
| [`cbuffer-produces-parameter-group-type-layout.slang`](cbuffer-produces-parameter-group-type-layout.slang)            | functional | `#global-scope-type-layout`      |
| [`entry-point-func-gets-layout-decoration.slang`](entry-point-func-gets-layout-decoration.slang)                 | functional | `#per-entry-point-steps`         |
| [`entry-point-layout-has-param-and-result-operands.slang`](entry-point-layout-has-param-and-result-operands.slang)        | functional | `#per-entry-point-steps`         |
| [`entry-point-layout-encodes-stage-and-semantic.slang`](entry-point-layout-encodes-stage-and-semantic.slang)           | functional | `#per-entry-point-steps`         |
| [`entry-point-func-keeps-export-decoration.slang`](entry-point-func-keeps-export-decoration.slang)                | functional | `#per-entry-point-steps`         |
| [`layout-is-per-target-spirv-vs-hlsl.slang`](layout-is-per-target-spirv-vs-hlsl.slang)                      | functional | `#why-this-module-exists`        |

## Doc gaps observed

- The doc states the capability-atom filter at lines 15462-15463 emits `IRRequireCapabilityAtomDecoration`s only for SPIR-V and Metal layout-module entry points, but does not name a textual surface (e.g. a `requireCapabilityAtom(...)` instruction or `[requireCapabilityAtom(...)]` decoration name) by which a reader could observe the decoration in `-dump-ir` output. Without that, comparing the post-link IR across SPIR-V vs HLSL targets is brittle (capability machinery is heavily rewritten by post-link passes). A doc sentence naming the IR opcode would unblock a focused test.
- The doc's "Per-entry-point steps" table row 6 mentions forwarding capability atoms in the range `[_spirv_1_0, latestSpirvAtom]` or `[metallib_2_3, latestMetalAtom]`, but the precise atom values and their string mangling in `-dump-ir` output is not given. Treat as a doc gap pending an example.
- The doc's "Optional obfuscation pass" describes the strip + DCE block but does not provide a user-observable consequence under `-obfuscate-code`. The doc could name the `IRNameHintDecoration` removal as the observable so a focused test could compare with/without `-obfuscate-code` flags.
- The doc's "Cache and reuse" section describes lazy construction (`getOrCreateIRModuleForLayout`) and the `m_irModuleForLayout` cache field, but cache hit/miss is not user-observable through `slangc` (the only signal is correctness across multiple compilations). No actionable doc gap, but worth noting that cache semantics are inherently untestable from this bundle.
- The doc's "Caveats and gotchas" lists `materialize` failure as `SLANG_UNEXPECTED("unhandled value flavor")` and the `SLANG_ASSERT(m_layout)` precondition; neither is reachable from user Slang source, so they cannot be tested. The doc could clarify that these are internal contract checks, not user-visible diagnostics.

## Out of scope (no-GPU runner)

- **Lazy construction / cache reuse** of `m_irModuleForLayout`: no user-visible signal through `slangc`.
- **Optional obfuscation pass**: gated on `-obfuscate-code`, observable only via obfuscated source-map machinery outside this bundle's scope.
- **Per-`TargetProgram` independence** (two layout modules in memory for two targets in one session): the cross-target shape difference is covered indirectly by `layout-is-per-target-spirv-vs-hlsl.slang` running on a separate invocation; in-process multi-target independence is not exposed through `slangc`.
- **`materialize` failure** (`SLANG_UNEXPECTED("unhandled value flavor")`): a hard crash for corrupted per-module IR caches; not user-reachable from Slang source.
- **`SLANG_ASSERT(m_layout)` precondition**: a hard assertion for callers that bypass the lazy accessor; not user-reachable.
- **`buildMangledNameToGlobalInstMap`** call at the end of `createIRModuleForLayout`: no user-observable consequence (mirrors the same out-of-scope finding in 04b).
- **`IRRequireCapabilityAtomDecoration` filter** for SPIR-V / Metal only: see doc gaps above; the post-link pipeline rewrites capability metadata heavily, so no clean surface signal exists in `-dump-ir` to distinguish layout-module-attached from executable-module-attached capability atoms.
- **No mandatory optimization passes run on the layout module**: the doc's "What this module is not" point that `constructSSA` / SCCP / CFG-simplify / DCE / early-inlining do not run on the layout side. The layout module's per-function bodies are stubs, so there is no positive textual signal; the negative signal (no SSA cleanup observable on a side that has no code) is unfalsifiable.

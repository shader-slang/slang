# Prompt: pipeline/04c-layout-ir.md

See [_common.md](_common.md) for the universal rules.

## Target

Produce `docs/llm-generated/pipeline/04c-layout-ir.md` — a
construction reference for the **layout IR module** built by
`TargetProgram::createIRModuleForLayout`
([slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp)
line ~15327). This module is **not** a copy of the executable
IR module; it is a sibling IR module whose only job is to carry
`IRLayoutDecoration`s on stub globals and entry points for the
target's chosen layout rules.

Audience: a compiler developer or tools author who needs to
understand how Slang materializes layout information into IR
form, why this module is per-target, and what it does and does
not contain.

## Page shape

This page does **not** use the four-phase decomposition. The
construction is straightforward enough to document as a linear
script with a single Mermaid flowchart and two reference tables.

### Required sections

1. `# Layout IR module construction`.
2. One-paragraph intro: this is the IR module returned by
   `TargetProgram::createIRModuleForLayout`. It exists once per
   `TargetProgram` instance and is cached on
   `m_irModuleForLayout`. It is **not** part of
   `linkAndOptimizeIR` and contains no executable code — only
   stub globals, stub entry-point functions, and
   `IRLayoutDecoration`s.
3. `## Source` — direct link to
   [slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp)
   line ~15327 and to
   [slang-target-program.h](../../../source/slang/slang-target-program.h)
   for the `m_irModuleForLayout` cache and `getOrCreateIRModuleForLayout`
   accessor.
4. `## Why this module exists` — short bulleted rationale:
   - Layout is target-specific. The same source program can have
     different binding numbers, register spaces, byte offsets,
     and entry-point parameter mappings for D3D11, D3D12, Vulkan,
     Metal, etc.
   - Carrying layout information **inside** an IR module makes
     it queryable by the reflection API and the linker.
   - Keeping it in a **separate** module avoids contaminating the
     per-translation-unit IR (which is cached on the `Module` and
     shared across all targets) with target-specific decoration.
5. `## When it is built` — short bulleted lifecycle:
   - Lazy. The first call to `getOrCreateIRModuleForLayout` on a
     `TargetProgram` builds it; subsequent calls return the
     cache.
   - The caller must ensure that `m_layout` (the `ProgramLayout`)
     has been computed first; the function `SLANG_ASSERT`s on
     this.
   - Built **after** semantic check and parameter binding, and
     after the per-module IR has been generated. It is **not**
     fed into `linkAndOptimizeIR`; it is consumed directly by
     reflection and by callers that need a layout-decorated view
     of the program.
6. `## Construction flow` — one Mermaid `flowchart TD` showing:
   `IRModule::create` → "global parameters loop"
   (`for varLayout in globalStructLayout->fields`) →
   "global-scope type layout" (`_lowerTypeLayoutCommon` and
   `ParameterGroupTypeLayout` handling) → "module-level
   `IRLayoutDecoration`" (on `irModule->getModuleInst()`) →
   "entry points loop" (`for entryPointLayout in programLayout->entryPoints`)
   → optional "obfuscation: strip + DCE" (gated on
   `shouldObfuscateCode()`) → `buildMangledNameToGlobalInstMap`
   → cache `m_irModuleForLayout`.
7. `## Per-global-parameter steps` — a small numbered table
   describing what the global-parameters loop does for each
   variable layout:

   | # | Step | Function | Notes |
   |---|------|----------|-------|
   | 1 | Materialize stub `IRGlobalVar` | `materialize(context, ensureDecl(context, varDecl.getDecl())).val` | Produces an `[import(...)]` stub if no definition is present |
   | 2 | Lower the variable layout | `lowerVarLayout(context, varLayout)` | Produces an `IRVarLayout` instruction |
   | 3 | Attach `IRLayoutDecoration` | `builder->addLayoutDecoration(irVar, irLayout)` | The layout becomes queryable on the stub |
   | 4 | Record in the global type-layout builder | `globalStructTypeLayoutBuilder.addField(irVar, irLayout)` | Feeds the module-level `IRStructTypeLayout` |
8. `## Global-scope type layout` — short paragraph + reference
   to the `ParameterGroupTypeLayout` branch. Explain that when
   the global scope is wrapped in a parameter group (a constant
   buffer / push-constant block), the module's layout
   decoration is an `IRParameterGroupTypeLayout` rather than the
   raw struct layout. Note `setContainerVarLayout`,
   `setElementVarLayout`, `setOffsetElementTypeLayout`.
9. `## Per-entry-point steps` — small numbered table:

   | # | Step | Function | Notes |
   |---|------|----------|-------|
   | 1 | Skip if no AST | `if (!funcDeclRef) continue;` | Deserialized entry points have no AST |
   | 2 | Skip unspecialized generics | `isUnspecializedGenericFuncDeclRef(funcDeclRef)` | Cannot produce a layout yet |
   | 3 | Lower the function type | `lowerType(context, getFuncType(astBuilder, funcDeclRef))` | Produces an `IRFuncType` |
   | 4 | Materialize the stub function | `emitDeclRef(context, funcDeclRef, irFuncType)` via `getSimpleVal` | Produces an `IRFunc` skeleton; usually `[import(...)]` |
   | 5 | Attach import linkage if missing | `builder->addImportDecoration(irFunc, mangledName)` | Wires the stub to its real implementation by mangled name |
   | 6 | Forward capability atoms | iterate `inferredCapabilityRequirements` and call `builder->addRequireCapabilityAtomDecoration` for SPIR-V (`_spirv_1_0..latestSpirvAtom`) and Metal (`metallib_2_3..latestMetalAtom`) atoms | Lets the layout module advertise the SPIR-V / Metal capability set per entry point |
   | 7 | Lower the entry-point layout | `lowerEntryPointLayout(context, entryPointLayout)` | Produces an `IREntryPointLayout` |
   | 8 | Attach `IRLayoutDecoration` | `builder->addLayoutDecoration(irFunc, irEntryPointLayout)` |
10. `## Optional obfuscation pass` — short subsection covering
    the `if (linkage->m_optionSet.shouldObfuscateCode())` block
    near line 15476:
    - `stripFrontEndOnlyInstructions` with
      `shouldStripNameHints = true` and `stripSourceLocs = true`.
    - `eliminateDeadCode` with `keepExportsAlive = true` and
      `keepLayoutsAlive = true`.
    - Why: when shipping an obfuscated module, layout
      information must not leak source-level names or locations.
11. `## What this module is not` — short bulleted contrast:
    - It is **not** the per-module IR (the IR cached on
      `Module::m_irModule` produced by
      `generateIRForTranslationUnit`).
    - It is **not** fed into `linkAndOptimizeIR` and does **not**
      go through the post-link target legalization pipelines
      (`target-pipelines/*.md`).
    - It has **no** function bodies for user-defined functions —
      the entry-point `IRFunc`s are stubs whose only purpose is
      to anchor an `IRLayoutDecoration` and (for SPIR-V/Metal)
      capability decorations.
    - The mandatory-optimization passes documented in
      [04b-pre-link-passes.md](../pipeline/04b-pre-link-passes.md)
      **do not run** here.
12. `## Cache and reuse` — short paragraph: the result is
    cached on `m_irModuleForLayout`, so a second call to
    `getOrCreateIRModuleForLayout` for the same target returns
    the same module. Different `TargetProgram` instances (one
    per target) build independent layout IR modules.
13. `## Caveats and gotchas`:
    - The global-parameters loop calls `SLANG_UNEXPECTED` if
      `materialize` fails to produce a value — a real failure
      mode if the per-module IR cache is corrupted.
    - The function asserts that `m_layout` is non-null; callers
      that bypass `getOrCreateIRModuleForLayout` and call
      `createIRModuleForLayout` directly will crash if the
      layout has not been built.
    - The capability decoration forward only covers SPIR-V and
      Metal atoms — other targets (HLSL, WGSL, CUDA) currently
      do not get capability decorations on layout-module entry
      points.
14. `## See also`:
    - [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)
    - [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md)
    - [../pipeline/04b-pre-link-passes.md](../pipeline/04b-pre-link-passes.md)
    - [../cross-cutting/targets.md](../cross-cutting/targets.md)
    - [../ir-reference/index.md](../ir-reference/index.md)
    - the target-pipeline index

## Scope and exclusions

- Do **not** turn this into another phased pipeline page. The
  construction is a linear loop-then-loop pattern; respect that.
- Do **not** restate the per-module pre-link pipeline; reference
  [04b-pre-link-passes.md](../pipeline/04b-pre-link-passes.md)
  instead.
- Do **not** document the reflection API surface here; that is
  cross-cutting reflection documentation territory.
- Be precise about line numbers: line ~15327 for the function
  start, line ~15476 for the obfuscation block, line ~15493 for
  `buildMangledNameToGlobalInstMap`.

## Quality checklist (in addition to the universal one)

- [ ] The page documents `createIRModuleForLayout` only — it
      does not slip into discussing `generateIRForTranslationUnit`
      or post-link passes.
- [ ] The two per-loop tables (global parameters, entry points)
      list every numbered step in source order.
- [ ] The obfuscation gate is shown only as the optional final
      block; it is not promoted into the main flow.
- [ ] The "What this module is not" section explicitly disclaims
      executable code, post-link passes, and mandatory
      optimization passes.
- [ ] All cited line numbers are within ±5 of the actual source
      locations.

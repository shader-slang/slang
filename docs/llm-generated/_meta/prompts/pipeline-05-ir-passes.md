# Prompt: pipeline/05-ir-passes.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/pipeline/05-ir-passes.md` — a categorized
inventory of the IR passes under `source/slang/slang-ir-*.cpp`, plus
the order in which they are typically applied between AST-to-IR
lowering and emission.

Audience: a developer adding a pass, debugging a miscompile, or
choosing where to insert a new transformation.

## Required structure

1. `# IR Passes` (title)
2. `## How the passes are ordered` — describe how
   [slang-emit.cpp](../../../source/slang/slang-emit.cpp) drives the
   sequence of IR transformations between lowering and target emit.
   Cite the function that orchestrates the pipeline (search the
   watched paths for the dispatch). State that the order is target-
   sensitive where applicable.
3. `## Pass categories` — group the ~325 passes under `slang-ir-*` into
   categories. Suggested categories:
   - **Cleanup / canonicalization** (DCE, simplify-cfg, peephole,
     eliminate-multilevel-break, ...)
   - **Specialization and generics** (specialize, defunctionalize,
     bind-existentials, generic substitution)
   - **Autodiff** (the `slang-ir-autodiff*` family)
   - **Validation and diagnostic passes** (`check-*`, `validate-*`)
   - **Lowering and legalization** (type-legalization, byte-address-
     legalize, buffer lowering, resource lowering, struct-field
     splitting)
   - **Target-specific lowering** (passes whose name encodes a target,
     e.g. `slang-ir-spirv-*`, `slang-ir-glsl-*`, `slang-ir-hlsl-*`,
     `slang-ir-metal-*`, `slang-ir-wgsl-*`, `slang-ir-cuda-*`)
   - **Inlining and call-graph manipulation** (inlining,
     call-graph, dll-export/import)
   - **Layout and binding** (`slang-ir-layout*`,
     `slang-ir-collect-global-uniforms`, `slang-ir-explicit-global-*`)
   - **Differentiation and coverage instrumentation**
     (`slang-ir-coverage-instrument`)
   For each category produce a table:

   ```markdown
   | Pass | File | Purpose |
   | --- | --- | --- |
   | DCE | [slang-ir-dce.cpp](../../../source/slang/slang-ir-dce.cpp) | Removes unused instructions |
   ```

   You do not need to list every single file; aim for representative
   members of each category and explicitly note that the category may
   contain additional files.
4. `## Adding a new pass` — short checklist (where to put the file,
   how to register it in the dispatcher, naming convention, common
   utilities such as `slang-ir-clone.h`,
   `slang-ir-dominators.h`).
5. `## Pass utilities` — point at shared helpers:
   `slang-ir-clone`, `slang-ir-dominators`, `slang-ir-call-graph`,
   `slang-ir-restructure-scoping`, etc.

## Quality checklist (in addition to the universal one)

- [ ] Every category section has at least one row whose file exists in
      the watched paths.
- [ ] No row claims a pass exists if no matching `slang-ir-*.cpp` is
      present in the watched paths.
- [ ] If a pass cannot be cleanly assigned to one category, place it in
      "Cleanup / canonicalization" or note explicitly that it is
      cross-cutting.
- [ ] Document length under 64 KB. If you cannot fit everything,
      summarize categories with a count and link to a representative
      file.

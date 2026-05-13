# Prompt: pipeline/overview.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/pipeline/overview.md` — a single-page
walk through the compilation pipeline from a `slangc` invocation to
emitted target code, naming each stage and the file(s) that drive it.

This document is the index for the per-stage documents under
[pipeline/](.). Detailed stage descriptions live in
`01-lex-preprocess.md` through `06-emit.md`; this overview is the
"map".

Audience: a developer who knows what a compiler is in general but does
not yet know how Slang slices its pipeline.

## Required structure

1. `# Compilation Pipeline Overview` (title)
2. `## End-to-end flow` — a mermaid `flowchart LR` diagram with one node
   per stage and arrows showing the typical data handed off:
   `Source -> Lex/Preprocess -> Parse -> Check -> AST-to-IR -> IR Passes -> Emit -> Target Artifact`.
3. `## Stages` — a level-2 subsection per stage, each:
   - one paragraph describing what enters and what leaves the stage,
   - a list of the primary files driving the stage (cite the watched
     paths),
   - a link to the corresponding detailed doc
     ([01-lex-preprocess.md](01-lex-preprocess.md), ..., [06-emit.md](06-emit.md)).
4. `## Driver entry points` — one paragraph linking the stages to
   `CompileRequest` / `Linkage` orchestration in
   [slang-compile-request.cpp](../../../source/slang/slang-compile-request.cpp)
   and the emit dispatcher in
   [slang-emit.cpp](../../../source/slang/slang-emit.cpp).
5. `## Cross-cutting concerns` — bullet list pointing to the
   [../cross-cutting/](../cross-cutting/) docs (diagnostics, IR
   instructions, targets, core module, serialization), each phrased as
   "follows the pipeline at every stage".

## Quality checklist (in addition to the universal one)

- [ ] No section duplicates content from a per-stage doc; the per-stage
      docs do the deep dive, this one is purely a roadmap.
- [ ] Mermaid diagram nodes use camelCase IDs and no explicit colors.
- [ ] Every stage subsection contains at least one file-path link to the
      watched paths.
- [ ] Document length under 24 KB.

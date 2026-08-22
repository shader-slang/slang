---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:41:10Z
target_doc: cross-cutting/core-module.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 6
actions:
  fixed: 5
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for cross-cutting/core-module.md

## Summary

Nothing was escalated: every observation that could be checked matched
the watched source, so no compiler defect is implied by this queue.
Five gaps were fixed with edits confined to the four sections they were
anchored to (`## Core module`, `### What the core module provides`,
`## Standard modules`, `## Preludes`), and one was rejected as bogus —
the GLSL-module gap rests on the premise that a `slangc` compile never
sets `enableGLSL`, which is false. Two of the fixed gaps were written
narrower than their `Suggested addition` asked, because the source
confirms only part of what the reporting agent proposed.

The narrowed cases are worth naming. Gap `8caf9bf5152a` asked for a
marking of which `syntax`-declared modifiers are front-end-only; only
the positive half (`globallycoherent` reaching HLSL / GLSL / SPIR-V)
is confirmed, and the negative half is likely wrong — `pervertex`
maps onto interpolation behaviour that is target-visible, and the
bundle contains no test that would have shown it. Gap `cdbd17dc974a`
asked for an `#include` excerpt as if it were the default emit shape;
it is not, it is what the _prelude override_ produces, so the new text
attributes it to the override the section already describes rather
than to the embedded prelude.

Two watched-path notes for the operator. The C++/CUDA prelude override
that produces the observed `#include` line lives in
`source/core/slang-test-tool-util.cpp` (`_addCPPPrelude`, lines 77-92)
and is invoked from `source/slangc/main.cpp:102-103`; the
`[ExperimentalModule]` gate lives in `source/slang/slang-session.cpp`
(lines 1767-1777) with its message in
`source/slang/slang-diagnostics.lua:469-474`. None of the four is in
`watched_paths`, which already carries a standing note about this in
the page's cache subsection. The page is now 23294 bytes against a
`size_cap` of 24576, so it is close to the cap; `lint` is clean.

## Actions

| Gap ID       | Action         | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    | Fix summary                                                                                                                                                                                             |
| ------------ | -------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 28966999842f | fixed          | `source/slang/hlsl.meta.slang:13692-13716` — the matrix-vector `mul` is a `__target_switch` with `glsl`/`metal`/`wgsl` = `"($1 * $0)"`, `hlsl` = `"mul"`, `spirv` = `OpVectorTimesMatrix`, and a Slang `default` body; `:10108-10131` (`dot` → `OpDot`) and `:12363-12377` (`length` → `OpExtInst ... glsl450 Length`). Emitted forms cross-checked against `hlsl-intrinsic-mul-per-target.slang`, `hlsl-intrinsic-dot-per-target.slang`, `hlsl-intrinsic-length-per-target.slang`.                                                                                                                                                                                                         | added a `__target_switch` paragraph plus a per-target `mul` table under `## Core module`, with the `dot`/`length` contrast in one sentence                                                              |
| fd3de5f33b5a | rejected-bogus | The observation misreads the compiler on both counts. `source/slangc/main.cpp:93` sets `desc.enableGLSL = true` for every command-line compile, so "a compile that never set `enableGLSL`" never describes `slangc`; and `source/slang/slang-options.cpp:1209` documents `-allow-glsl` as "Enable GLSL as an input language", a parser/checker switch (`slang-parser.cpp:9985`, `slang-check-modifier.cpp:1957`), not a module loader. `source/slang/slang-session.cpp:1547-1565` shows there is no on-demand load either: an unloaded module diagnoses `GlslModuleNotAvailable` and does not fall back to a user `glsl.slang`, so the document's existing sentence is accurate as written. | —                                                                                                                                                                                                       |
| cdbd17dc974a | fixed          | `prelude/slang-cpp-prelude.h:52` defines `SLANG_PRELUDE_EXPORT`; the `#include` shape is produced by the override path `source/core/slang-test-tool-util.cpp:88-91`, reached from `source/slangc/main.cpp:102-103`, not by the embedded prelude. Emit shape confirmed by `prelude-cpp-referenced-from-emit.slang` and `prelude-cuda-referenced-from-emit.slang`.                                                                                                                                                                                                                                                                                                                            | added a three-line `-target cpp` excerpt under `## Preludes`, attributed to the prelude override the section already describes, naming `SLANG_PRELUDE_EXPORT` and the CUDA counterpart                  |
| 983c8f195742 | fixed          | `source/slang/core.meta.slang:128` declares `attribute_syntax [ExperimentalModule]`; `source/standard-modules/neural/neural.slang:21` and `source/standard-modules/experimental/workgraph.slang:9` carry it. The gate and message are `source/slang/slang-session.cpp:1767-1777` and `source/slang/slang-diagnostics.lua:469-474` (code 104), matching the CHECK in `standard-module-neural-experimental-gate.slang`.                                                                                                                                                                                                                                                                       | added a paragraph after the standard-module bullets stating that `[ExperimentalModule]` gates the import and names `-experimental-feature`                                                              |
| 8caf9bf5152a | fixed          | `source/slang/core.meta.slang:32` (`syntax globallycoherent : GloballyCoherentModifier;`); emitted forms from `core-globallycoherent-syntax-modifier.slang` (HLSL `globallycoherent`, GLSL `coherent`, SPIR-V `OpDecorate ... Coherent`). The gap's front-end-only claim for `constexpr` / `pervertex` was not written — no watched-path or test evidence supports it.                                                                                                                                                                                                                                                                                                                      | added one sentence to `### What the core module provides` recording that `syntax`-declared modifiers can reach emit, with `globallycoherent` as the confirmed example; consolidated with `04664b3f7ad6` |
| 04664b3f7ad6 | fixed          | `source/slang/core.meta.slang:1822-1847` (`Optional<T>`: `hasValue` / `value` properties, implicit conversion from `T`, `__init()` = `none`), `:1384-1386` (`__none_t`), `:1929-1949` (`Tuple` positional `_0`/`_1`, swizzle concatenation, `makeTuple`), `:116-126` (`IRangedValue` with `static const This maxValue` / `minValue`) and the per-scalar extensions at `:1643`, `:2226`. Member usage cross-checked against `core-optional-and-tuple-without-import.slang` and `core-irangedvalue-generic-extensions.slang`.                                                                                                                                                                 | added a three-bullet member sketch for `Optional`, `Tuple` and `IRangedValue` to `### What the core module provides`; consolidated with `8caf9bf5152a`                                                  |

---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:33+00:00
target_doc: cross-cutting/core-module.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 6654bd6fd16af6b2f9308c3f9d577016f106721659f147496571fe0a0cefd1f4
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: pass
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: pass
  front_matter_validity: pass
finding_count: 0
severity_breakdown:
  critical: 0
  major: 0
  minor: 0
  nit: 0
---

# Review report for cross-cutting/core-module.md

## Summary
The page passes the review checklist. It covers the required shipped-code families, core and GLSL module embedding, standard modules, preludes, and build options, and the source spot checks matched the current watched files.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show cross-cutting/core-module.md` and reviewed the per-doc prompt, common prompt, manifest entry, resolved watched files, and dependency document `architecture/overview.md`.
- Checked front matter for all required keys, the recorded source commit, the warning string, and a 64-character hex watched-path digest.
- Checked relative links in the document body, including meta-Slang sources, embedding glue, standard-module files, prelude headers, generated peer docs, `CLAUDE.md`, and user-guide links; no dangling links were found in the final lint run.
- Verified the required sections: `## What ships with the compiler`, `## Core module`, `## GLSL module`, `## Standard modules`, `## Preludes`, and `## Building the core module`.
- Spot-checked more than 10 source claims, including all four watched `*.meta.slang` files, `public module core`, core scalar typedefs, `__intrinsic_op` mappings, HLSL texture/buffer declarations, `IDifferentiable` / `DifferentialPair`, GLSL aliases and `gl_*` values, core and GLSL embedding CMake targets, `SLANG_EMBED_CORE_MODULE` wiring, the standard-module README, the `neural` module, and every resolved prelude header.
- Checked that no body line-number citations needed verification; the document uses file links without source line anchors.

## Findings
(no findings)

## No-issues notes
- The document mentions every resolved meta-Slang source and every resolved `prelude/*.h` header.
- The standard-module section correctly identifies `neural` as the only module under `source/standard-modules/` at this source commit.
- The build-option description matches both `CLAUDE.md` and the CMake generator-expression wiring.

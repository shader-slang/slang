---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:07:46+00:00
target_doc: pipeline/04b-pre-link-passes.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 0a827687878ad7390b1acdda49546f652c2eaf5da2d820809834f1faa2ed69cb
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for pipeline/04b-pre-link-passes.md

## Summary

The document substantially matches the recorded source and satisfies the required four-phase contract. Three issues remain: it names a nonexistent `Module::compile` caller, overstates the contents of the layout IR module, and records a watched-path digest that does not match the watched files at its own `source_commit`. The fabricated caller is the most important finding because it directs readers to an API that is not present in the source tree.

## Items checked

- Read the target document, `_common.md`, the per-doc prompt, and all four dependency documents.
- Resolved all five watched files with `regenerate.py show` and verified source against commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Spot-checked more than 30 factual claims covering all four phases, pass ordering and gates, the early-inlining loop, prelink cloning, stripping, obfuscation, linker caches, and adjacent module constructors.
- Verified every line-number citation across 65 citation-bearing lines; the cited definitions and ranges match the recorded commit.
- Resolved all 49 distinct linked source, generated-document, and directory paths at the recorded commit; the two local heading anchors also exist.
- Checked the mandatory sections, tables, diagrams, gate groups, notable-pass callouts, three adjacent constructs, size cap, front-matter keys, and review style rules.

## Findings

| ID    | Severity | Location                                                    | Description                                                                                                                                                                                                                                                                                                                           | Evidence                                                                                                                                                                                                                                                                                                                                              | Recommendation                                                                                                                                                                                                                                             |
| ----- | -------- | ----------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | major    | `## Source`, lines 31-35                                    | The document says `generateIRForTranslationUnit` is invoked from `Module::compile`, but no `Module::compile` symbol exists. The normal translation-unit driver is `FrontEndCompileRequest::generateIR`; imported modules use a separate linkage loading path.                                                                         | `source/slang/slang-compile-request.cpp:526-570` defines `FrontEndCompileRequest::generateIR`, calls `generateIRForTranslationUnit` at line 552, and caches the result at line 570. `source/slang/slang-session.cpp:1123-1131` shows the imported-module call path.                                                                                   | Replace “from `Module::compile` and friends” with the two real callers: `FrontEndCompileRequest::generateIR` for request translation units and the imported-module generation path in `slang-session.cpp`.                                                 |
| F-002 | minor    | `### TargetProgram::createIRModuleForLayout`, lines 627-635 | The claim that the layout module's “only contents are `IRLayoutDecoration`s on stub globals and entry points” is too strong. The constructor also attaches a layout decoration to the module instruction, emits layout/type instructions, adds import linkage to entry-point stubs, and forwards SPIR-V/Metal capability decorations. | `source/slang/slang-lower-to-ir.cpp:16412-16450` builds variable and module-level layouts; lines 16470-16498 add import, capability, and layout decorations to entry-point stubs.                                                                                                                                                                     | Say that the separate module carries target layout metadata on import stubs and the module instruction, with capability decorations on applicable entry points; retain that it contains no executable bodies and does not run the mandatory pass sequence. |
| F-003 | minor    | YAML front-matter, lines 1-7                                | `watched_paths_digest` is valid hexadecimal but does not match the digest of the five resolved watched files at the recorded `source_commit`. The document records `0a827687...ed69cb`; recomputation from that commit yields `6b4337c0...1603b1`.                                                                                    | `docs/generated/design/_meta/regenerate.py:441-457` defines the digest over path, size, and contents. Running `regenerate.py digest pipeline/04b-pre-link-passes.md` at HEAD, which equals the recorded source commit, and independently hashing the five commit blobs both yield `6b4337c01022716f64c1f958ec59f2ab400c9c07e4d2646c5bea3a771e1603b1`. | Refresh the generated document's `watched_paths_digest` through the regeneration/remediation workflow so it records `6b4337c01022716f64c1f958ec59f2ab400c9c07e4d2646c5bea3a771e1603b1`; do not hand-edit the ledger.                                       |

## No-issues notes

- The direct pre-link calls and all conditional gates match `generateIRForTranslationUnit`.
- The loop section accurately captures the subtle overwrite of `changed` by `peepholeOptimizeGlobalScope`.
- The revised `obfuscateModuleLocs` text correctly states that locations remain unchanged when obfuscation is enabled without a source map.
- The `prelinkIR` cloning, auto-diff pruning, and stable-input linking-cache descriptions align with `slang-ir-link.cpp`.

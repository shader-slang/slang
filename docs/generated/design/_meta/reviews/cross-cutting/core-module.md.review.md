---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:24:00+00:00
target_doc: cross-cutting/core-module.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 2c33d82801bf8c85c90f7a72974d48339879a5c470ed66dd9ad1279eeab52e62
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for cross-cutting/core-module.md

## Summary

The document largely matches the prompt contract and the watched source files: it covers the core, HLSL, GLSL, diff meta-slang files, the neural standard module, and all resolved prelude headers. The only issue I found is a small but real source-alignment error in the GLSL module loading description: the source gates loading on the global-session `enableGLSL` flag, not directly on the current target or an on-demand request for GLSL-flavored names.

## Items checked

- Verified the required front matter fields and copied `target_doc_source_commit` / `target_doc_watched_paths_digest` from the target document.
- Used `regenerate.py show cross-cutting/core-module.md` to identify the prompt, dependency document, watched paths, and resolved watched files.
- Read `_common.md`, `cross-cutting-core-module.md`, the target document, and the dependency document `architecture/overview.md`.
- Spot-checked more than 10 concrete claims against resolved watched files: `core.meta.slang`, `hlsl.meta.slang`, `diff.meta.slang`, `glsl.meta.slang`, `source/slang-core-module/CMakeLists.txt`, the embedded core / GLSL module glue, `source/standard-modules/README.md`, `source/standard-modules/CMakeLists.txt`, `source/standard-modules/neural/CMakeLists.txt`, `neural.slang`, `slang-standard-module-config.h.in`, and every resolved `prelude/*.h`.
- Resolved all 43 Markdown links in the target document; none were missing.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## GLSL module`, lines 121-123 | The statement `Loading it is target-conditional: the compiler pulls it in when the user is compiling GLSL or asks for GLSL-flavoured names from Slang code` describes the trigger too narrowly. The source loads the GLSL builtin module during global session creation when `desc->enableGLSL` is true; explicit `import glsl` then retrieves that builtin module if it is available. | `source/slang/slang-api.cpp:218` gates GLSL loading with `if (desc->enableGLSL)`, and `source/slang/slang-session.cpp:1520` handles `moduleName == getSessionImpl()->glslModuleName` by returning `getBuiltinModule(BuiltinModuleName::GLSL)`. `include/slang.h:5657` documents `enableGLSL` as the global-session flag. | Replace the sentence with wording tied to `SlangGlobalSessionDesc::enableGLSL`, for example: "The global session loads the GLSL builtin module when `enableGLSL` is set; later GLSL-language scopes or `import glsl` use that loaded builtin module." |

## No-issues notes

- The standard-module build description matches `source/standard-modules/neural/CMakeLists.txt`: the module is built with `slang-bootstrap` and `-load-core-module ${core_module_archive}`.
- The core-module build-product description matches `source/slang-core-module/CMakeLists.txt`: one `-compile-core-module` command writes the core archive plus core and GLSL embeddable headers.
- The prelude table covers all resolved `prelude/*.h` files from the manifest output.

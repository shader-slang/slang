# tests-agentic — bundle index

Navigational table of contents for every bundle in `tests-agentic/`. Each row links
to that bundle's `README.md` (front-matter + claims/tests/doc-gaps index) and
to the documentation file that anchors its tests.

See [`README.md`](README.md) for the framework intro and the trust model.
See [`_meta/regenerate.md`](_meta/regenerate.md) for the operator workflow.

## Suite totals

- **Bundles:** 52
- **Total `.slang` tests:** 2652

| Intent | Count |
| --- | --- |
| `functional` | 916 |
| `boundary` | 632 |
| `negative` | 590 |
| `expansion` | 402 |
| `stress` | 111 |
| `regression` | 1 |

## Bundles by section

### Architecture

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`architecture/dependency-graph`](architecture/dependency-graph/README.md) | 5 | [`docs/llm-generated/architecture/dependency-graph.md`](../docs/llm-generated/architecture/dependency-graph.md) |
| [`architecture/module-map`](architecture/module-map/README.md) | 6 | [`docs/llm-generated/architecture/module-map.md`](../docs/llm-generated/architecture/module-map.md) |
| [`architecture/overview`](architecture/overview/README.md) | 5 | [`docs/llm-generated/architecture/overview.md`](../docs/llm-generated/architecture/overview.md) |

### Pipeline

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`pipeline/01-lex-preprocess`](pipeline/01-lex-preprocess/README.md) | 43 | [`docs/llm-generated/pipeline/01-lex-preprocess.md`](../docs/llm-generated/pipeline/01-lex-preprocess.md) |
| [`pipeline/02-parse-ast`](pipeline/02-parse-ast/README.md) | 20 | [`docs/llm-generated/pipeline/02-parse-ast.md`](../docs/llm-generated/pipeline/02-parse-ast.md) |
| [`pipeline/03-semantic-check`](pipeline/03-semantic-check/README.md) | 77 | [`docs/llm-generated/pipeline/03-semantic-check.md`](../docs/llm-generated/pipeline/03-semantic-check.md) |
| [`pipeline/04-ast-to-ir`](pipeline/04-ast-to-ir/README.md) | 125 | [`docs/llm-generated/pipeline/04-ast-to-ir.md`](../docs/llm-generated/pipeline/04-ast-to-ir.md) |
| [`pipeline/04b-pre-link-passes`](pipeline/04b-pre-link-passes/README.md) | 16 | [`docs/llm-generated/pipeline/04b-pre-link-passes.md`](../docs/llm-generated/pipeline/04b-pre-link-passes.md) |
| [`pipeline/04c-layout-ir`](pipeline/04c-layout-ir/README.md) | 13 | [`docs/llm-generated/pipeline/04c-layout-ir.md`](../docs/llm-generated/pipeline/04c-layout-ir.md) |
| [`pipeline/05-ir-passes`](pipeline/05-ir-passes/README.md) | 109 | [`docs/llm-generated/pipeline/05-ir-passes.md`](../docs/llm-generated/pipeline/05-ir-passes.md) |
| [`pipeline/06-emit`](pipeline/06-emit/README.md) | 39 | [`docs/llm-generated/pipeline/06-emit.md`](../docs/llm-generated/pipeline/06-emit.md) |
| [`pipeline/overview`](pipeline/overview/README.md) | 7 | [`docs/llm-generated/pipeline/overview.md`](../docs/llm-generated/pipeline/overview.md) |

### Syntax reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`syntax-reference/grammar`](syntax-reference/grammar/README.md) | 37 | [`docs/llm-generated/syntax-reference/grammar.md`](../docs/llm-generated/syntax-reference/grammar.md) |
| [`syntax-reference/keywords-and-builtins`](syntax-reference/keywords-and-builtins/README.md) | 54 | [`docs/llm-generated/syntax-reference/keywords-and-builtins.md`](../docs/llm-generated/syntax-reference/keywords-and-builtins.md) |
| [`syntax-reference/tokens`](syntax-reference/tokens/README.md) | 34 | [`docs/llm-generated/syntax-reference/tokens.md`](../docs/llm-generated/syntax-reference/tokens.md) |

### Cross-cutting

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`cross-cutting/core-module`](cross-cutting/core-module/README.md) | 77 | [`docs/llm-generated/cross-cutting/core-module.md`](../docs/llm-generated/cross-cutting/core-module.md) |
| [`cross-cutting/diagnostics`](cross-cutting/diagnostics/README.md) | 50 | [`docs/llm-generated/cross-cutting/diagnostics.md`](../docs/llm-generated/cross-cutting/diagnostics.md) |
| [`cross-cutting/diagnostics-catalog`](cross-cutting/diagnostics-catalog/README.md) | 323 | [`docs/llm-generated/cross-cutting/diagnostics.md`](../docs/llm-generated/cross-cutting/diagnostics.md) |
| [`cross-cutting/ir-instructions`](cross-cutting/ir-instructions/README.md) | 125 | [`docs/llm-generated/cross-cutting/ir-instructions.md`](../docs/llm-generated/cross-cutting/ir-instructions.md) |
| [`cross-cutting/serialization`](cross-cutting/serialization/README.md) | 9 | [`docs/llm-generated/cross-cutting/serialization.md`](../docs/llm-generated/cross-cutting/serialization.md) |
| [`cross-cutting/targets`](cross-cutting/targets/README.md) | 34 | [`docs/llm-generated/cross-cutting/targets.md`](../docs/llm-generated/cross-cutting/targets.md) |

### AST reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`ast-reference/base`](ast-reference/base/README.md) | 12 | [`docs/llm-generated/ast-reference/base.md`](../docs/llm-generated/ast-reference/base.md) |
| [`ast-reference/declarations`](ast-reference/declarations/README.md) | 46 | [`docs/llm-generated/ast-reference/declarations.md`](../docs/llm-generated/ast-reference/declarations.md) |
| [`ast-reference/expressions`](ast-reference/expressions/README.md) | 103 | [`docs/llm-generated/ast-reference/expressions.md`](../docs/llm-generated/ast-reference/expressions.md) |
| [`ast-reference/index`](ast-reference/index/README.md) | 5 | [`docs/llm-generated/ast-reference/index.md`](../docs/llm-generated/ast-reference/index.md) |
| [`ast-reference/modifiers`](ast-reference/modifiers/README.md) | 24 | [`docs/llm-generated/ast-reference/modifiers.md`](../docs/llm-generated/ast-reference/modifiers.md) |
| [`ast-reference/statements`](ast-reference/statements/README.md) | 50 | [`docs/llm-generated/ast-reference/statements.md`](../docs/llm-generated/ast-reference/statements.md) |
| [`ast-reference/types`](ast-reference/types/README.md) | 100 | [`docs/llm-generated/ast-reference/types.md`](../docs/llm-generated/ast-reference/types.md) |
| [`ast-reference/values`](ast-reference/values/README.md) | 11 | [`docs/llm-generated/ast-reference/values.md`](../docs/llm-generated/ast-reference/values.md) |

### Name resolution

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`name-resolution/index`](name-resolution/index/README.md) | 6 | [`docs/llm-generated/name-resolution/index.md`](../docs/llm-generated/name-resolution/index.md) |
| [`name-resolution/lookup`](name-resolution/lookup/README.md) | 24 | [`docs/llm-generated/name-resolution/lookup.md`](../docs/llm-generated/name-resolution/lookup.md) |
| [`name-resolution/overload-resolution`](name-resolution/overload-resolution/README.md) | 24 | [`docs/llm-generated/name-resolution/overload-resolution.md`](../docs/llm-generated/name-resolution/overload-resolution.md) |
| [`name-resolution/scopes`](name-resolution/scopes/README.md) | 25 | [`docs/llm-generated/name-resolution/scopes.md`](../docs/llm-generated/name-resolution/scopes.md) |
| [`name-resolution/visibility`](name-resolution/visibility/README.md) | 16 | [`docs/llm-generated/name-resolution/visibility.md`](../docs/llm-generated/name-resolution/visibility.md) |

### IR reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`ir-reference/control-flow`](ir-reference/control-flow/README.md) | 37 | [`docs/llm-generated/ir-reference/control-flow.md`](../docs/llm-generated/ir-reference/control-flow.md) |
| [`ir-reference/decorations`](ir-reference/decorations/README.md) | 62 | [`docs/llm-generated/ir-reference/decorations.md`](../docs/llm-generated/ir-reference/decorations.md) |
| [`ir-reference/differentiation`](ir-reference/differentiation/README.md) | 16 | [`docs/llm-generated/ir-reference/differentiation.md`](../docs/llm-generated/ir-reference/differentiation.md) |
| [`ir-reference/generics-and-existentials`](ir-reference/generics-and-existentials/README.md) | 39 | [`docs/llm-generated/ir-reference/generics-and-existentials.md`](../docs/llm-generated/ir-reference/generics-and-existentials.md) |
| [`ir-reference/index`](ir-reference/index/README.md) | 6 | [`docs/llm-generated/ir-reference/index.md`](../docs/llm-generated/ir-reference/index.md) |
| [`ir-reference/metadata`](ir-reference/metadata/README.md) | 22 | [`docs/llm-generated/ir-reference/metadata.md`](../docs/llm-generated/ir-reference/metadata.md) |
| [`ir-reference/misc`](ir-reference/misc/README.md) | 10 | [`docs/llm-generated/ir-reference/misc.md`](../docs/llm-generated/ir-reference/misc.md) |
| [`ir-reference/resources-and-atomics`](ir-reference/resources-and-atomics/README.md) | 85 | [`docs/llm-generated/ir-reference/resources-and-atomics.md`](../docs/llm-generated/ir-reference/resources-and-atomics.md) |
| [`ir-reference/structure`](ir-reference/structure/README.md) | 22 | [`docs/llm-generated/ir-reference/structure.md`](../docs/llm-generated/ir-reference/structure.md) |
| [`ir-reference/types`](ir-reference/types/README.md) | 115 | [`docs/llm-generated/ir-reference/types.md`](../docs/llm-generated/ir-reference/types.md) |
| [`ir-reference/values`](ir-reference/values/README.md) | 124 | [`docs/llm-generated/ir-reference/values.md`](../docs/llm-generated/ir-reference/values.md) |

### Target pipelines

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`target-pipelines/cuda`](target-pipelines/cuda/README.md) | 55 | [`docs/llm-generated/target-pipelines/cuda.md`](../docs/llm-generated/target-pipelines/cuda.md) |
| [`target-pipelines/hlsl`](target-pipelines/hlsl/README.md) | 135 | [`docs/llm-generated/target-pipelines/hlsl.md`](../docs/llm-generated/target-pipelines/hlsl.md) |
| [`target-pipelines/index`](target-pipelines/index/README.md) | 5 | [`docs/llm-generated/target-pipelines/index.md`](../docs/llm-generated/target-pipelines/index.md) |
| [`target-pipelines/metal`](target-pipelines/metal/README.md) | 66 | [`docs/llm-generated/target-pipelines/metal.md`](../docs/llm-generated/target-pipelines/metal.md) |
| [`target-pipelines/spirv`](target-pipelines/spirv/README.md) | 126 | [`docs/llm-generated/target-pipelines/spirv.md`](../docs/llm-generated/target-pipelines/spirv.md) |
| [`target-pipelines/wgsl`](target-pipelines/wgsl/README.md) | 59 | [`docs/llm-generated/target-pipelines/wgsl.md`](../docs/llm-generated/target-pipelines/wgsl.md) |

### Top-level

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`glossary`](glossary/README.md) | 14 | [`docs/llm-generated/glossary.md`](../docs/llm-generated/glossary.md) |

## Catalog snapshot

- [`_meta/diagnostics-catalog/catalog.txt`](_meta/diagnostics-catalog/catalog.txt) — full diagnostic-code catalog (695 entries) consumed by the `cross-cutting/diagnostics-catalog` bundle.

## Conventions

- Every bundle's `README.md` carries YAML front-matter with `generated_at`, `source_commit`, `watched_paths_digest`, `source_doc_digest`, plus a `## Claims enumerated` and `## Tests in this bundle` table.
- Each `.slang` test file starts with a `//META` block declaring `doc_ref`, `intent`, `pipeline_stage`, and provenance.
- Bundles are agent-generated. Hand-editing a `README.md` or a `.slang` file is an anti-pattern — file a doc-improvement or prompt-improvement task and regenerate.


# docs/generated/tests — bundle index

Navigational table of contents for every bundle in `docs/generated/tests/`. Each row links
to that bundle's `README.md` and to the documentation file that anchors its tests.

See [`README.md`](README.md) for the framework intro and the trust model.
See [`_meta/regenerate.md`](_meta/regenerate.md) for the operator workflow.

## Suite totals

- **Bundles:** 44
- **Total `.slang` tests:** 2683

| Intent | Count |
| --- | --- |
| `functional` | 910 |
| `boundary` | 632 |
| `negative` | 590 |
| `expansion` | 439 |
| `stress` | 111 |
| `regression` | 1 |

## Bundles by section

### Pipeline

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`pipeline/01-lex-preprocess`](pipeline/01-lex-preprocess/README.md) | 43 | [`docs/generated/design/pipeline/01-lex-preprocess.md`](../design/pipeline/01-lex-preprocess.md) |
| [`pipeline/02-parse-ast`](pipeline/02-parse-ast/README.md) | 20 | [`docs/generated/design/pipeline/02-parse-ast.md`](../design/pipeline/02-parse-ast.md) |
| [`pipeline/03-semantic-check`](pipeline/03-semantic-check/README.md) | 77 | [`docs/generated/design/pipeline/03-semantic-check.md`](../design/pipeline/03-semantic-check.md) |
| [`pipeline/04-ast-to-ir`](pipeline/04-ast-to-ir/README.md) | 125 | [`docs/generated/design/pipeline/04-ast-to-ir.md`](../design/pipeline/04-ast-to-ir.md) |
| [`pipeline/04b-pre-link-passes`](pipeline/04b-pre-link-passes/README.md) | 16 | [`docs/generated/design/pipeline/04b-pre-link-passes.md`](../design/pipeline/04b-pre-link-passes.md) |
| [`pipeline/04c-layout-ir`](pipeline/04c-layout-ir/README.md) | 13 | [`docs/generated/design/pipeline/04c-layout-ir.md`](../design/pipeline/04c-layout-ir.md) |
| [`pipeline/05-ir-passes`](pipeline/05-ir-passes/README.md) | 109 | [`docs/generated/design/pipeline/05-ir-passes.md`](../design/pipeline/05-ir-passes.md) |
| [`pipeline/06-emit`](pipeline/06-emit/README.md) | 42 | [`docs/generated/design/pipeline/06-emit.md`](../design/pipeline/06-emit.md) |
| [`pipeline/overview`](pipeline/overview/README.md) | 7 | [`docs/generated/design/pipeline/overview.md`](../design/pipeline/overview.md) |

### Syntax reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`syntax-reference/grammar`](syntax-reference/grammar/README.md) | 37 | [`docs/generated/design/syntax-reference/grammar.md`](../design/syntax-reference/grammar.md) |
| [`syntax-reference/keywords-and-builtins`](syntax-reference/keywords-and-builtins/README.md) | 56 | [`docs/generated/design/syntax-reference/keywords-and-builtins.md`](../design/syntax-reference/keywords-and-builtins.md) |
| [`syntax-reference/tokens`](syntax-reference/tokens/README.md) | 34 | [`docs/generated/design/syntax-reference/tokens.md`](../design/syntax-reference/tokens.md) |

### Cross-cutting

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`cross-cutting/core-module`](cross-cutting/core-module/README.md) | 77 | [`docs/generated/design/cross-cutting/core-module.md`](../design/cross-cutting/core-module.md) |
| [`cross-cutting/diagnostics`](cross-cutting/diagnostics/README.md) | 50 | [`docs/generated/design/cross-cutting/diagnostics.md`](../design/cross-cutting/diagnostics.md) |
| [`cross-cutting/diagnostics-catalog`](cross-cutting/diagnostics-catalog/README.md) | 323 | [`docs/generated/design/cross-cutting/diagnostics.md`](../design/cross-cutting/diagnostics.md) |
| [`cross-cutting/ir-instructions`](cross-cutting/ir-instructions/README.md) | 125 | [`docs/generated/design/cross-cutting/ir-instructions.md`](../design/cross-cutting/ir-instructions.md) |
| [`cross-cutting/serialization`](cross-cutting/serialization/README.md) | 10 | [`docs/generated/design/cross-cutting/serialization.md`](../design/cross-cutting/serialization.md) |
| [`cross-cutting/targets`](cross-cutting/targets/README.md) | 36 | [`docs/generated/design/cross-cutting/targets.md`](../design/cross-cutting/targets.md) |

### AST reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`ast-reference/base`](ast-reference/base/README.md) | 12 | [`docs/generated/design/ast-reference/base.md`](../design/ast-reference/base.md) |
| [`ast-reference/declarations`](ast-reference/declarations/README.md) | 46 | [`docs/generated/design/ast-reference/declarations.md`](../design/ast-reference/declarations.md) |
| [`ast-reference/expressions`](ast-reference/expressions/README.md) | 103 | [`docs/generated/design/ast-reference/expressions.md`](../design/ast-reference/expressions.md) |
| [`ast-reference/modifiers`](ast-reference/modifiers/README.md) | 29 | [`docs/generated/design/ast-reference/modifiers.md`](../design/ast-reference/modifiers.md) |
| [`ast-reference/statements`](ast-reference/statements/README.md) | 50 | [`docs/generated/design/ast-reference/statements.md`](../design/ast-reference/statements.md) |
| [`ast-reference/types`](ast-reference/types/README.md) | 109 | [`docs/generated/design/ast-reference/types.md`](../design/ast-reference/types.md) |
| [`ast-reference/values`](ast-reference/values/README.md) | 11 | [`docs/generated/design/ast-reference/values.md`](../design/ast-reference/values.md) |

### Name resolution

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`name-resolution/lookup`](name-resolution/lookup/README.md) | 24 | [`docs/generated/design/name-resolution/lookup.md`](../design/name-resolution/lookup.md) |
| [`name-resolution/overload-resolution`](name-resolution/overload-resolution/README.md) | 24 | [`docs/generated/design/name-resolution/overload-resolution.md`](../design/name-resolution/overload-resolution.md) |
| [`name-resolution/scopes`](name-resolution/scopes/README.md) | 25 | [`docs/generated/design/name-resolution/scopes.md`](../design/name-resolution/scopes.md) |
| [`name-resolution/visibility`](name-resolution/visibility/README.md) | 16 | [`docs/generated/design/name-resolution/visibility.md`](../design/name-resolution/visibility.md) |

### IR reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`ir-reference/control-flow`](ir-reference/control-flow/README.md) | 37 | [`docs/generated/design/ir-reference/control-flow.md`](../design/ir-reference/control-flow.md) |
| [`ir-reference/decorations`](ir-reference/decorations/README.md) | 64 | [`docs/generated/design/ir-reference/decorations.md`](../design/ir-reference/decorations.md) |
| [`ir-reference/differentiation`](ir-reference/differentiation/README.md) | 16 | [`docs/generated/design/ir-reference/differentiation.md`](../design/ir-reference/differentiation.md) |
| [`ir-reference/generics-and-existentials`](ir-reference/generics-and-existentials/README.md) | 39 | [`docs/generated/design/ir-reference/generics-and-existentials.md`](../design/ir-reference/generics-and-existentials.md) |
| [`ir-reference/metadata`](ir-reference/metadata/README.md) | 24 | [`docs/generated/design/ir-reference/metadata.md`](../design/ir-reference/metadata.md) |
| [`ir-reference/misc`](ir-reference/misc/README.md) | 10 | [`docs/generated/design/ir-reference/misc.md`](../design/ir-reference/misc.md) |
| [`ir-reference/resources-and-atomics`](ir-reference/resources-and-atomics/README.md) | 99 | [`docs/generated/design/ir-reference/resources-and-atomics.md`](../design/ir-reference/resources-and-atomics.md) |
| [`ir-reference/structure`](ir-reference/structure/README.md) | 22 | [`docs/generated/design/ir-reference/structure.md`](../design/ir-reference/structure.md) |
| [`ir-reference/types`](ir-reference/types/README.md) | 125 | [`docs/generated/design/ir-reference/types.md`](../design/ir-reference/types.md) |
| [`ir-reference/values`](ir-reference/values/README.md) | 127 | [`docs/generated/design/ir-reference/values.md`](../design/ir-reference/values.md) |

### Target pipelines

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`target-pipelines/cuda`](target-pipelines/cuda/README.md) | 61 | [`docs/generated/design/target-pipelines/cuda.md`](../design/target-pipelines/cuda.md) |
| [`target-pipelines/hlsl`](target-pipelines/hlsl/README.md) | 143 | [`docs/generated/design/target-pipelines/hlsl.md`](../design/target-pipelines/hlsl.md) |
| [`target-pipelines/metal`](target-pipelines/metal/README.md) | 73 | [`docs/generated/design/target-pipelines/metal.md`](../design/target-pipelines/metal.md) |
| [`target-pipelines/spirv`](target-pipelines/spirv/README.md) | 132 | [`docs/generated/design/target-pipelines/spirv.md`](../design/target-pipelines/spirv.md) |
| [`target-pipelines/wgsl`](target-pipelines/wgsl/README.md) | 62 | [`docs/generated/design/target-pipelines/wgsl.md`](../design/target-pipelines/wgsl.md) |

## Catalog snapshot

- [`_meta/diagnostics-catalog/catalog.txt`](_meta/diagnostics-catalog/catalog.txt) — full diagnostic-code catalog consumed by the `cross-cutting/diagnostics-catalog` bundle.

## Conventions

- Every bundle's `README.md` carries YAML front-matter (`generated_at`, `source_commit`, `watched_paths_digest`, `source_doc_digest`) and four canonical sections: `## Intent`, `## Functional coverage`, `## Untested claims`, `## Doc gaps observed`.
- Each `.slang` test file starts with a `//META` block declaring `doc_ref`, `intent`, `pipeline_stage`, and provenance.
- Bundles are agent-generated. Hand-editing a `README.md` or a `.slang` file is an anti-pattern — file a doc-improvement or prompt-improvement task and regenerate.
- Regenerate this file with `python3 docs/generated/tests/_meta/regenerate.py index --write`.

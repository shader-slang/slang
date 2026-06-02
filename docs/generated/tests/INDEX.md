# docs/generated/tests — bundle index

Navigational table of contents for every bundle in `docs/generated/tests/`. Each row links
to that bundle's `README.md` and to the documentation file that anchors its tests.

See [`README.md`](README.md) for the framework intro and the trust model.
See [`_meta/regenerate.md`](_meta/regenerate.md) for the operator workflow.

## Suite totals

- **Bundles:** 53
- **Total `.slang` tests:** 2748

| Intent       | Count |
| ------------ | ----- |
| `functional` | 945   |
| `boundary`   | 663   |
| `negative`   | 591   |
| `expansion`  | 437   |
| `stress`     | 111   |
| `regression` | 1     |

## Bundles by section

### Spec (language reference)

| Bundle                                                                       | Tests | Source doc                                                                                                      |
| ---------------------------------------------------------------------------- | ----: | --------------------------------------------------------------------------------------------------------------- |
| [`spec/expressions-literal`](spec/expressions-literal/README.md)             |    12 | [`docs/language-reference/expressions-literal.md`](../../language-reference/expressions-literal.md)             |
| [`spec/expressions-member-access`](spec/expressions-member-access/README.md) |     5 | [`docs/language-reference/expressions-member-access.md`](../../language-reference/expressions-member-access.md) |
| [`spec/expressions-operators`](spec/expressions-operators/README.md)         |    10 | [`docs/language-reference/expressions-operators.md`](../../language-reference/expressions-operators.md)         |
| [`spec/lexical-structure`](spec/lexical-structure/README.md)                 |     8 | [`docs/language-reference/lexical-structure.md`](../../language-reference/lexical-structure.md)                 |
| [`spec/types-array`](spec/types-array/README.md)                             |     6 | [`docs/language-reference/types-array.md`](../../language-reference/types-array.md)                             |
| [`spec/types-enum`](spec/types-enum/README.md)                               |     5 | [`docs/language-reference/types-enum.md`](../../language-reference/types-enum.md)                               |
| [`spec/types-extension`](spec/types-extension/README.md)                     |     5 | [`docs/language-reference/types-extension.md`](../../language-reference/types-extension.md)                     |
| [`spec/types-fundamental-integer`](spec/types-fundamental-integer/README.md) |     8 | [`docs/language-reference/types-fundamental.md`](../../language-reference/types-fundamental.md)                 |
| [`spec/types-vector-and-matrix`](spec/types-vector-and-matrix/README.md)     |     9 | [`docs/language-reference/types-vector-and-matrix.md`](../../language-reference/types-vector-and-matrix.md)     |

### Pipeline

| Bundle                                                                                         | Tests | Source doc                                                                                           |
| ---------------------------------------------------------------------------------------------- | ----: | ---------------------------------------------------------------------------------------------------- |
| [`regression/pipeline/01-lex-preprocess`](regression/pipeline/01-lex-preprocess/README.md)     |    43 | [`docs/generated/design/pipeline/01-lex-preprocess.md`](../design/pipeline/01-lex-preprocess.md)     |
| [`regression/pipeline/02-parse-ast`](regression/pipeline/02-parse-ast/README.md)               |    20 | [`docs/generated/design/pipeline/02-parse-ast.md`](../design/pipeline/02-parse-ast.md)               |
| [`regression/pipeline/03-semantic-check`](regression/pipeline/03-semantic-check/README.md)     |    77 | [`docs/generated/design/pipeline/03-semantic-check.md`](../design/pipeline/03-semantic-check.md)     |
| [`regression/pipeline/04-ast-to-ir`](regression/pipeline/04-ast-to-ir/README.md)               |   125 | [`docs/generated/design/pipeline/04-ast-to-ir.md`](../design/pipeline/04-ast-to-ir.md)               |
| [`regression/pipeline/04b-pre-link-passes`](regression/pipeline/04b-pre-link-passes/README.md) |    16 | [`docs/generated/design/pipeline/04b-pre-link-passes.md`](../design/pipeline/04b-pre-link-passes.md) |
| [`regression/pipeline/04c-layout-ir`](regression/pipeline/04c-layout-ir/README.md)             |    13 | [`docs/generated/design/pipeline/04c-layout-ir.md`](../design/pipeline/04c-layout-ir.md)             |
| [`regression/pipeline/05-ir-passes`](regression/pipeline/05-ir-passes/README.md)               |   109 | [`docs/generated/design/pipeline/05-ir-passes.md`](../design/pipeline/05-ir-passes.md)               |
| [`regression/pipeline/06-emit`](regression/pipeline/06-emit/README.md)                         |    42 | [`docs/generated/design/pipeline/06-emit.md`](../design/pipeline/06-emit.md)                         |
| [`regression/pipeline/overview`](regression/pipeline/overview/README.md)                       |     7 | [`docs/generated/design/pipeline/overview.md`](../design/pipeline/overview.md)                       |

### Syntax reference

| Bundle                                                                                                             | Tests | Source doc                                                                                                               |
| ------------------------------------------------------------------------------------------------------------------ | ----: | ------------------------------------------------------------------------------------------------------------------------ |
| [`regression/syntax-reference/grammar`](regression/syntax-reference/grammar/README.md)                             |    37 | [`docs/generated/design/syntax-reference/grammar.md`](../design/syntax-reference/grammar.md)                             |
| [`regression/syntax-reference/keywords-and-builtins`](regression/syntax-reference/keywords-and-builtins/README.md) |    56 | [`docs/generated/design/syntax-reference/keywords-and-builtins.md`](../design/syntax-reference/keywords-and-builtins.md) |
| [`regression/syntax-reference/tokens`](regression/syntax-reference/tokens/README.md)                               |    34 | [`docs/generated/design/syntax-reference/tokens.md`](../design/syntax-reference/tokens.md)                               |

### Cross-cutting

| Bundle                                                                                                   | Tests | Source doc                                                                                             |
| -------------------------------------------------------------------------------------------------------- | ----: | ------------------------------------------------------------------------------------------------------ |
| [`regression/cross-cutting/core-module`](regression/cross-cutting/core-module/README.md)                 |    77 | [`docs/generated/design/cross-cutting/core-module.md`](../design/cross-cutting/core-module.md)         |
| [`regression/cross-cutting/diagnostics`](regression/cross-cutting/diagnostics/README.md)                 |    50 | [`docs/generated/design/cross-cutting/diagnostics.md`](../design/cross-cutting/diagnostics.md)         |
| [`regression/cross-cutting/diagnostics-catalog`](regression/cross-cutting/diagnostics-catalog/README.md) |   323 | [`docs/generated/design/cross-cutting/diagnostics.md`](../design/cross-cutting/diagnostics.md)         |
| [`regression/cross-cutting/ir-instructions`](regression/cross-cutting/ir-instructions/README.md)         |   125 | [`docs/generated/design/cross-cutting/ir-instructions.md`](../design/cross-cutting/ir-instructions.md) |
| [`regression/cross-cutting/serialization`](regression/cross-cutting/serialization/README.md)             |     9 | [`docs/generated/design/cross-cutting/serialization.md`](../design/cross-cutting/serialization.md)     |
| [`regression/cross-cutting/targets`](regression/cross-cutting/targets/README.md)                         |    36 | [`docs/generated/design/cross-cutting/targets.md`](../design/cross-cutting/targets.md)                 |

### AST reference

| Bundle                                                                                     | Tests | Source doc                                                                                       |
| ------------------------------------------------------------------------------------------ | ----: | ------------------------------------------------------------------------------------------------ |
| [`regression/ast-reference/base`](regression/ast-reference/base/README.md)                 |    12 | [`docs/generated/design/ast-reference/base.md`](../design/ast-reference/base.md)                 |
| [`regression/ast-reference/declarations`](regression/ast-reference/declarations/README.md) |    46 | [`docs/generated/design/ast-reference/declarations.md`](../design/ast-reference/declarations.md) |
| [`regression/ast-reference/expressions`](regression/ast-reference/expressions/README.md)   |   103 | [`docs/generated/design/ast-reference/expressions.md`](../design/ast-reference/expressions.md)   |
| [`regression/ast-reference/modifiers`](regression/ast-reference/modifiers/README.md)       |    29 | [`docs/generated/design/ast-reference/modifiers.md`](../design/ast-reference/modifiers.md)       |
| [`regression/ast-reference/statements`](regression/ast-reference/statements/README.md)     |    50 | [`docs/generated/design/ast-reference/statements.md`](../design/ast-reference/statements.md)     |
| [`regression/ast-reference/types`](regression/ast-reference/types/README.md)               |   109 | [`docs/generated/design/ast-reference/types.md`](../design/ast-reference/types.md)               |
| [`regression/ast-reference/values`](regression/ast-reference/values/README.md)             |    11 | [`docs/generated/design/ast-reference/values.md`](../design/ast-reference/values.md)             |

### Name resolution

| Bundle                                                                                                       | Tests | Source doc                                                                                                         |
| ------------------------------------------------------------------------------------------------------------ | ----: | ------------------------------------------------------------------------------------------------------------------ |
| [`regression/name-resolution/lookup`](regression/name-resolution/lookup/README.md)                           |    24 | [`docs/generated/design/name-resolution/lookup.md`](../design/name-resolution/lookup.md)                           |
| [`regression/name-resolution/overload-resolution`](regression/name-resolution/overload-resolution/README.md) |    24 | [`docs/generated/design/name-resolution/overload-resolution.md`](../design/name-resolution/overload-resolution.md) |
| [`regression/name-resolution/scopes`](regression/name-resolution/scopes/README.md)                           |    25 | [`docs/generated/design/name-resolution/scopes.md`](../design/name-resolution/scopes.md)                           |
| [`regression/name-resolution/visibility`](regression/name-resolution/visibility/README.md)                   |    16 | [`docs/generated/design/name-resolution/visibility.md`](../design/name-resolution/visibility.md)                   |

### IR reference

| Bundle                                                                                                             | Tests | Source doc                                                                                                               |
| ------------------------------------------------------------------------------------------------------------------ | ----: | ------------------------------------------------------------------------------------------------------------------------ |
| [`regression/ir-reference/control-flow`](regression/ir-reference/control-flow/README.md)                           |    37 | [`docs/generated/design/ir-reference/control-flow.md`](../design/ir-reference/control-flow.md)                           |
| [`regression/ir-reference/decorations`](regression/ir-reference/decorations/README.md)                             |    64 | [`docs/generated/design/ir-reference/decorations.md`](../design/ir-reference/decorations.md)                             |
| [`regression/ir-reference/differentiation`](regression/ir-reference/differentiation/README.md)                     |    16 | [`docs/generated/design/ir-reference/differentiation.md`](../design/ir-reference/differentiation.md)                     |
| [`regression/ir-reference/generics-and-existentials`](regression/ir-reference/generics-and-existentials/README.md) |    39 | [`docs/generated/design/ir-reference/generics-and-existentials.md`](../design/ir-reference/generics-and-existentials.md) |
| [`regression/ir-reference/metadata`](regression/ir-reference/metadata/README.md)                                   |    24 | [`docs/generated/design/ir-reference/metadata.md`](../design/ir-reference/metadata.md)                                   |
| [`regression/ir-reference/misc`](regression/ir-reference/misc/README.md)                                           |    10 | [`docs/generated/design/ir-reference/misc.md`](../design/ir-reference/misc.md)                                           |
| [`regression/ir-reference/resources-and-atomics`](regression/ir-reference/resources-and-atomics/README.md)         |    99 | [`docs/generated/design/ir-reference/resources-and-atomics.md`](../design/ir-reference/resources-and-atomics.md)         |
| [`regression/ir-reference/structure`](regression/ir-reference/structure/README.md)                                 |    22 | [`docs/generated/design/ir-reference/structure.md`](../design/ir-reference/structure.md)                                 |
| [`regression/ir-reference/types`](regression/ir-reference/types/README.md)                                         |   125 | [`docs/generated/design/ir-reference/types.md`](../design/ir-reference/types.md)                                         |
| [`regression/ir-reference/values`](regression/ir-reference/values/README.md)                                       |   127 | [`docs/generated/design/ir-reference/values.md`](../design/ir-reference/values.md)                                       |

### Target pipelines

| Bundle                                                                             | Tests | Source doc                                                                               |
| ---------------------------------------------------------------------------------- | ----: | ---------------------------------------------------------------------------------------- |
| [`regression/target-pipelines/cuda`](regression/target-pipelines/cuda/README.md)   |    61 | [`docs/generated/design/target-pipelines/cuda.md`](../design/target-pipelines/cuda.md)   |
| [`regression/target-pipelines/hlsl`](regression/target-pipelines/hlsl/README.md)   |   143 | [`docs/generated/design/target-pipelines/hlsl.md`](../design/target-pipelines/hlsl.md)   |
| [`regression/target-pipelines/metal`](regression/target-pipelines/metal/README.md) |    72 | [`docs/generated/design/target-pipelines/metal.md`](../design/target-pipelines/metal.md) |
| [`regression/target-pipelines/spirv`](regression/target-pipelines/spirv/README.md) |   131 | [`docs/generated/design/target-pipelines/spirv.md`](../design/target-pipelines/spirv.md) |
| [`regression/target-pipelines/wgsl`](regression/target-pipelines/wgsl/README.md)   |    62 | [`docs/generated/design/target-pipelines/wgsl.md`](../design/target-pipelines/wgsl.md)   |

## Catalog snapshot

- [`_meta/diagnostics-catalog/catalog.txt`](_meta/diagnostics-catalog/catalog.txt) — full diagnostic-code catalog consumed by the `cross-cutting/diagnostics-catalog` bundle.

## Conventions

- Every bundle's `README.md` carries YAML front-matter (`generated_at`, `source_commit`, `watched_paths_digest`, `source_doc_digest`) and four canonical sections: `## Intent`, `## Functional coverage`, `## Untested claims`, `## Doc gaps observed`.
- Each `.slang` test file starts with a `//META` block declaring `doc_ref`, `intent`, `pipeline_stage`, and provenance.
- Bundles are agent-generated. Hand-editing a `README.md` or a `.slang` file is an anti-pattern — file a doc-improvement or prompt-improvement task and regenerate.
- Regenerate this file with `python3 docs/generated/tests/_meta/regenerate.py index --write`.

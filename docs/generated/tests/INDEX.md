# docs/generated/tests — bundle index

Navigational table of contents for every bundle in `docs/generated/tests/`. Each row links
to that bundle's `README.md` and to the documentation file that anchors its tests.

See [`README.md`](README.md) for the framework intro and the trust model.
See [`_meta/regenerate.md`](_meta/regenerate.md) for the operator workflow.

## Suite totals

- **Bundles:** 79
- **Total `.slang` tests:** 4103

| Intent | Count |
| --- | --- |
| `functional` | 1936 |
| `negative` | 791 |
| `boundary` | 775 |
| `expansion` | 299 |
| `characterization` | 215 |
| `stress` | 84 |
| `regression` | 3 |

## Bundles by section

### Conformance (language reference)

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`conformance/basics-execution-divergence-reconvergence`](conformance/basics-execution-divergence-reconvergence/README.md) | 8 | [`docs/language-reference/basics-execution-divergence-reconvergence.md`](../../language-reference/basics-execution-divergence-reconvergence.md) |
| [`conformance/basics-memory-model-address-spaces`](conformance/basics-memory-model-address-spaces/README.md) | 20 | [`docs/language-reference/basics-memory-model-address-spaces.md`](../../language-reference/basics-memory-model-address-spaces.md) |
| [`conformance/basics-memory-model-consistency`](conformance/basics-memory-model-consistency/README.md) | 20 | [`docs/language-reference/basics-memory-model-consistency.md`](../../language-reference/basics-memory-model-consistency.md) |
| [`conformance/basics-memory-model-special-topics`](conformance/basics-memory-model-special-topics/README.md) | 17 | [`docs/language-reference/basics-memory-model-special-topics.md`](../../language-reference/basics-memory-model-special-topics.md) |
| [`conformance/basics-program-execution`](conformance/basics-program-execution/README.md) | 10 | [`docs/language-reference/basics-program-execution.md`](../../language-reference/basics-program-execution.md) |
| [`conformance/declarations`](conformance/declarations/README.md) | 43 | [`docs/language-reference/declarations.md`](../../language-reference/declarations.md) |
| [`conformance/expressions-identifier`](conformance/expressions-identifier/README.md) | 10 | [`docs/language-reference/expressions-identifier.md`](../../language-reference/expressions-identifier.md) |
| [`conformance/expressions-initializer`](conformance/expressions-initializer/README.md) | 10 | [`docs/language-reference/expressions-initializer.md`](../../language-reference/expressions-initializer.md) |
| [`conformance/expressions-literal`](conformance/expressions-literal/README.md) | 12 | [`docs/language-reference/expressions-literal.md`](../../language-reference/expressions-literal.md) |
| [`conformance/expressions-member-access`](conformance/expressions-member-access/README.md) | 5 | [`docs/language-reference/expressions-member-access.md`](../../language-reference/expressions-member-access.md) |
| [`conformance/expressions-operator-precedence`](conformance/expressions-operator-precedence/README.md) | 25 | [`docs/language-reference/expressions-operator-precedence.md`](../../language-reference/expressions-operator-precedence.md) |
| [`conformance/expressions-operators`](conformance/expressions-operators/README.md) | 10 | [`docs/language-reference/expressions-operators.md`](../../language-reference/expressions-operators.md) |
| [`conformance/expressions-this`](conformance/expressions-this/README.md) | 6 | [`docs/language-reference/expressions-this.md`](../../language-reference/expressions-this.md) |
| [`conformance/generics`](conformance/generics/README.md) | 60 | [`docs/language-reference/generics.md`](../../language-reference/generics.md) |
| [`conformance/lexical-structure`](conformance/lexical-structure/README.md) | 8 | [`docs/language-reference/lexical-structure.md`](../../language-reference/lexical-structure.md) |
| [`conformance/statements`](conformance/statements/README.md) | 36 | [`docs/language-reference/statements.md`](../../language-reference/statements.md) |
| [`conformance/types-array`](conformance/types-array/README.md) | 6 | [`docs/language-reference/types-array.md`](../../language-reference/types-array.md) |
| [`conformance/types-enum`](conformance/types-enum/README.md) | 5 | [`docs/language-reference/types-enum.md`](../../language-reference/types-enum.md) |
| [`conformance/types-extension`](conformance/types-extension/README.md) | 5 | [`docs/language-reference/types-extension.md`](../../language-reference/types-extension.md) |
| [`conformance/types-fundamental-integer`](conformance/types-fundamental-integer/README.md) | 8 | [`docs/language-reference/types-fundamental.md`](../../language-reference/types-fundamental.md) |
| [`conformance/types-interface`](conformance/types-interface/README.md) | 33 | [`docs/language-reference/types-interface.md`](../../language-reference/types-interface.md) |
| [`conformance/types-pointer`](conformance/types-pointer/README.md) | 19 | [`docs/language-reference/types-pointer.md`](../../language-reference/types-pointer.md) |
| [`conformance/types-struct`](conformance/types-struct/README.md) | 31 | [`docs/language-reference/types-struct.md`](../../language-reference/types-struct.md) |
| [`conformance/types-vector-and-matrix`](conformance/types-vector-and-matrix/README.md) | 9 | [`docs/language-reference/types-vector-and-matrix.md`](../../language-reference/types-vector-and-matrix.md) |

### Pipeline

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`design/pipeline/01-lex-preprocess`](design/pipeline/01-lex-preprocess/README.md) | 75 | [`docs/generated/design/pipeline/01-lex-preprocess.md`](../design/pipeline/01-lex-preprocess.md) |
| [`design/pipeline/02-parse-ast`](design/pipeline/02-parse-ast/README.md) | 76 | [`docs/generated/design/pipeline/02-parse-ast.md`](../design/pipeline/02-parse-ast.md) |
| [`design/pipeline/03-semantic-check`](design/pipeline/03-semantic-check/README.md) | 91 | [`docs/generated/design/pipeline/03-semantic-check.md`](../design/pipeline/03-semantic-check.md) |
| [`design/pipeline/04-ast-to-ir`](design/pipeline/04-ast-to-ir/README.md) | 160 | [`docs/generated/design/pipeline/04-ast-to-ir.md`](../design/pipeline/04-ast-to-ir.md) |
| [`design/pipeline/04b-pre-link-passes`](design/pipeline/04b-pre-link-passes/README.md) | 37 | [`docs/generated/design/pipeline/04b-pre-link-passes.md`](../design/pipeline/04b-pre-link-passes.md) |
| [`design/pipeline/04c-layout-ir`](design/pipeline/04c-layout-ir/README.md) | 34 | [`docs/generated/design/pipeline/04c-layout-ir.md`](../design/pipeline/04c-layout-ir.md) |
| [`design/pipeline/05-ir-passes`](design/pipeline/05-ir-passes/README.md) | 144 | [`docs/generated/design/pipeline/05-ir-passes.md`](../design/pipeline/05-ir-passes.md) |
| [`design/pipeline/06-emit`](design/pipeline/06-emit/README.md) | 39 | [`docs/generated/design/pipeline/06-emit.md`](../design/pipeline/06-emit.md) |
| [`design/pipeline/overview`](design/pipeline/overview/README.md) | 15 | [`docs/generated/design/pipeline/overview.md`](../design/pipeline/overview.md) |

### Syntax reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`design/syntax-reference/grammar`](design/syntax-reference/grammar/README.md) | 97 | [`docs/generated/design/syntax-reference/grammar.md`](../design/syntax-reference/grammar.md) |
| [`design/syntax-reference/keywords-and-builtins`](design/syntax-reference/keywords-and-builtins/README.md) | 60 | [`docs/generated/design/syntax-reference/keywords-and-builtins.md`](../design/syntax-reference/keywords-and-builtins.md) |
| [`design/syntax-reference/tokens`](design/syntax-reference/tokens/README.md) | 50 | [`docs/generated/design/syntax-reference/tokens.md`](../design/syntax-reference/tokens.md) |

### Cross-cutting

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`design/cross-cutting/core-module`](design/cross-cutting/core-module/README.md) | 32 | [`docs/generated/design/cross-cutting/core-module.md`](../design/cross-cutting/core-module.md) |
| [`design/cross-cutting/diagnostics`](design/cross-cutting/diagnostics/README.md) | 65 | [`docs/generated/design/cross-cutting/diagnostics.md`](../design/cross-cutting/diagnostics.md) |
| [`design/cross-cutting/diagnostics-catalog`](design/cross-cutting/diagnostics-catalog/README.md) | 339 | [`docs/generated/design/cross-cutting/diagnostics.md`](../design/cross-cutting/diagnostics.md) |
| [`design/cross-cutting/ir-instructions`](design/cross-cutting/ir-instructions/README.md) | 52 | [`docs/generated/design/cross-cutting/ir-instructions.md`](../design/cross-cutting/ir-instructions.md) |
| [`design/cross-cutting/serialization`](design/cross-cutting/serialization/README.md) | 13 | [`docs/generated/design/cross-cutting/serialization.md`](../design/cross-cutting/serialization.md) |
| [`design/cross-cutting/targets`](design/cross-cutting/targets/README.md) | 42 | [`docs/generated/design/cross-cutting/targets.md`](../design/cross-cutting/targets.md) |

### AST reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`design/ast-reference/base`](design/ast-reference/base/README.md) | 24 | [`docs/generated/design/ast-reference/base.md`](../design/ast-reference/base.md) |
| [`design/ast-reference/declarations`](design/ast-reference/declarations/README.md) | 84 | [`docs/generated/design/ast-reference/declarations.md`](../design/ast-reference/declarations.md) |
| [`design/ast-reference/expressions`](design/ast-reference/expressions/README.md) | 144 | [`docs/generated/design/ast-reference/expressions.md`](../design/ast-reference/expressions.md) |
| [`design/ast-reference/modifiers`](design/ast-reference/modifiers/README.md) | 62 | [`docs/generated/design/ast-reference/modifiers.md`](../design/ast-reference/modifiers.md) |
| [`design/ast-reference/statements`](design/ast-reference/statements/README.md) | 70 | [`docs/generated/design/ast-reference/statements.md`](../design/ast-reference/statements.md) |
| [`design/ast-reference/types`](design/ast-reference/types/README.md) | 49 | [`docs/generated/design/ast-reference/types.md`](../design/ast-reference/types.md) |
| [`design/ast-reference/values`](design/ast-reference/values/README.md) | 31 | [`docs/generated/design/ast-reference/values.md`](../design/ast-reference/values.md) |

### Name resolution

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`design/name-resolution/lookup`](design/name-resolution/lookup/README.md) | 53 | [`docs/generated/design/name-resolution/lookup.md`](../design/name-resolution/lookup.md) |
| [`design/name-resolution/overload-resolution`](design/name-resolution/overload-resolution/README.md) | 90 | [`docs/generated/design/name-resolution/overload-resolution.md`](../design/name-resolution/overload-resolution.md) |
| [`design/name-resolution/scopes`](design/name-resolution/scopes/README.md) | 57 | [`docs/generated/design/name-resolution/scopes.md`](../design/name-resolution/scopes.md) |
| [`design/name-resolution/visibility`](design/name-resolution/visibility/README.md) | 42 | [`docs/generated/design/name-resolution/visibility.md`](../design/name-resolution/visibility.md) |

### IR reference

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`design/ir-reference/control-flow`](design/ir-reference/control-flow/README.md) | 60 | [`docs/generated/design/ir-reference/control-flow.md`](../design/ir-reference/control-flow.md) |
| [`design/ir-reference/decorations`](design/ir-reference/decorations/README.md) | 78 | [`docs/generated/design/ir-reference/decorations.md`](../design/ir-reference/decorations.md) |
| [`design/ir-reference/differentiation`](design/ir-reference/differentiation/README.md) | 36 | [`docs/generated/design/ir-reference/differentiation.md`](../design/ir-reference/differentiation.md) |
| [`design/ir-reference/generics-and-existentials`](design/ir-reference/generics-and-existentials/README.md) | 60 | [`docs/generated/design/ir-reference/generics-and-existentials.md`](../design/ir-reference/generics-and-existentials.md) |
| [`design/ir-reference/metadata`](design/ir-reference/metadata/README.md) | 69 | [`docs/generated/design/ir-reference/metadata.md`](../design/ir-reference/metadata.md) |
| [`design/ir-reference/misc`](design/ir-reference/misc/README.md) | 30 | [`docs/generated/design/ir-reference/misc.md`](../design/ir-reference/misc.md) |
| [`design/ir-reference/resources-and-atomics`](design/ir-reference/resources-and-atomics/README.md) | 108 | [`docs/generated/design/ir-reference/resources-and-atomics.md`](../design/ir-reference/resources-and-atomics.md) |
| [`design/ir-reference/structure`](design/ir-reference/structure/README.md) | 47 | [`docs/generated/design/ir-reference/structure.md`](../design/ir-reference/structure.md) |
| [`design/ir-reference/types`](design/ir-reference/types/README.md) | 150 | [`docs/generated/design/ir-reference/types.md`](../design/ir-reference/types.md) |
| [`design/ir-reference/values`](design/ir-reference/values/README.md) | 160 | [`docs/generated/design/ir-reference/values.md`](../design/ir-reference/values.md) |

### Target pipelines

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`design/target-pipelines/cuda`](design/target-pipelines/cuda/README.md) | 93 | [`docs/generated/design/target-pipelines/cuda.md`](../design/target-pipelines/cuda.md) |
| [`design/target-pipelines/hlsl`](design/target-pipelines/hlsl/README.md) | 170 | [`docs/generated/design/target-pipelines/hlsl.md`](../design/target-pipelines/hlsl.md) |
| [`design/target-pipelines/metal`](design/target-pipelines/metal/README.md) | 78 | [`docs/generated/design/target-pipelines/metal.md`](../design/target-pipelines/metal.md) |
| [`design/target-pipelines/spirv`](design/target-pipelines/spirv/README.md) | 125 | [`docs/generated/design/target-pipelines/spirv.md`](../design/target-pipelines/spirv.md) |
| [`design/target-pipelines/wgsl`](design/target-pipelines/wgsl/README.md) | 81 | [`docs/generated/design/target-pipelines/wgsl.md`](../design/target-pipelines/wgsl.md) |

### Coverage (white-box characterization)

| Bundle | Tests | Source doc |
| --- | ---: | --- |
| [`coverage/autodiff`](coverage/autodiff/README.md) | 9 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/check-decl`](coverage/check-decl/README.md) | 29 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/check-expr`](coverage/check-expr/README.md) | 22 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/cli-options`](coverage/cli-options/README.md) | 30 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/emit`](coverage/emit/README.md) | 13 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/legalize`](coverage/legalize/README.md) | 23 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/lower-to-ir`](coverage/lower-to-ir/README.md) | 19 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/parser`](coverage/parser/README.md) | 30 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/specialize`](coverage/specialize/README.md) | 10 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/torch`](coverage/torch/README.md) | 8 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |
| [`coverage/type-layout`](coverage/type-layout/README.md) | 22 | [`coverage/METHODOLOGY.md`](coverage/METHODOLOGY.md) |

## Catalog snapshot

- [`_meta/diagnostics-catalog/catalog.txt`](_meta/diagnostics-catalog/catalog.txt) — full diagnostic-code catalog consumed by the `cross-cutting/diagnostics-catalog` bundle.

## Conventions

- Every bundle's `README.md` carries YAML front-matter (`generated_at`, `source_commit`, `watched_paths_digest`, `source_doc_digest`) and four canonical sections: `## Intent`, `## Functional coverage`, `## Untested claims`, `## Doc gaps observed`.
- Each `.slang` test file starts with a `//META` block declaring `doc_ref`, `intent`, `pipeline_stage`, and provenance.
- Bundles are agent-generated. Hand-editing a `README.md` or a `.slang` file is an anti-pattern — file a doc-improvement or prompt-improvement task and regenerate.
- Regenerate this file with `python3 docs/generated/tests/_meta/regenerate.py index --write`.

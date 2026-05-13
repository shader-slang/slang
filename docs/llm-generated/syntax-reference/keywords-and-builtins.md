---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-07T14:35:56+00:00
source_commit: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
watched_paths_digest: 22bb179e0c05c42e0fa5a3d9bd14d5b619b443751d7badca33af72dad5d5a748
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Keywords and Built-in Syntax

This document inventories Slang's syntactic keywords. The non-obvious
fact it must convey: most "keywords" are not lexer-level reserved
words. They arrive at the parser as `TokenType::Identifier` (see
[tokens.md](tokens.md)) and become keywords only because a
`SyntaxDecl` in the active environment binds them to a parsing
callback. Adding or renaming a keyword therefore touches the parser's
syntax table or the core-module sources, not the lexer.

The intended reader is a developer adding a new keyword or trying to
understand why a specific identifier is special.

## Where keywords come from

Three sources contribute keywords:

1. **Hardcoded statement keywords** in
   [slang-parser.cpp](../../../source/slang/slang-parser.cpp). The
   statement parser inspects identifiers via `LookAheadToken("if")`,
   `LookAheadToken("for")`, etc., and dispatches to dedicated parse
   functions.
2. **The parser's `SyntaxParseInfo` table**
   ([slang-parser.cpp](../../../source/slang/slang-parser.cpp), the
   array `g_parseSyntaxEntries[]`), populated by
   `getSyntaxParseInfos()`. This is the source of decl, modifier, and
   expression keywords.
3. **Core-module `*.meta.slang` declarations**. The meta-modules
   ([core.meta.slang](../../../source/slang/core.meta.slang),
   [hlsl.meta.slang](../../../source/slang/hlsl.meta.slang),
   [glsl.meta.slang](../../../source/slang/glsl.meta.slang),
   [diff.meta.slang](../../../source/slang/diff.meta.slang)) declare
   built-in types, functions, and operators that contribute names to
   the default environment. These are processed at compiler startup
   and the resulting decls behave like keywords for purposes of
   parser disambiguation.

## Lexer-recognized symbols

The lexer does *not* recognize alphabetic keywords. The only tokens
spelled out in the lexer / token catalog are punctuation and operators
— see [tokens.md](tokens.md) for the full list (`Semicolon`,
`Scope (::)`, `RightArrow (->)`, `DoubleRightArrow (=>)`, all the
`Op*` operators, etc.).

## Statement keywords

Recognized in the statement parser by direct identifier comparison.
Cited line numbers refer to
[slang-parser.cpp](../../../source/slang/slang-parser.cpp) at
`source_commit`.

| Keyword | Where parsed |
| --- | --- |
| `if` | line 6358 (`LookAheadToken("if")`); `else` follow at lines 6782, 6819 |
| `for` | lines 6298, 6340, 6369, 6859 (header parsing and statement entry) |
| `while` | lines 6371, 6898, 6911, 6958 |
| `do` | line 6373 |
| `break` | lines 6375, 6979 |
| `continue` | lines 6377, 6992 |
| `return` | lines 6379, 7001 |
| `switch` | lines 6014, 6388 |
| `case` | lines 6057, 6087, 6396 |
| `default` | lines 6063, 6087, 6398 |
| `discard` | line 6381 |
| `defer` | line 6406 |
| `throw` | line 6414 |
| `catch` | lines 6945, 6964 (paired with `try`) |

These keywords are not in the syntax-decl table because Slang treats
control-flow as a closed grammar; they cannot be redefined by user
code.

## Decl keywords

Registered in `g_parseSyntaxEntries[]` at
[slang-parser.cpp](../../../source/slang/slang-parser.cpp) line 10170
through `_makeParseDecl(...)`. Identifiers that begin with double
underscore (`__`) are intentionally namespaced as compiler-internal /
non-stable.

| Keyword | Parses |
| --- | --- |
| `typedef` | C-style type alias (`parseTypeDef`) |
| `typealias` | Slang-style type alias (`parseTypeAliasDecl`) |
| `associatedtype` | Interface associated type (`parseAssocType`) |
| `__associatedfunc` | Interface associated function (`parseAssocFunc`) |
| `type_param` | Module-level generic type parameter (`parseGlobalGenericTypeParamDecl`) |
| `__generic` | Generic-parameter list head (`parseGenericDecl`) |
| `__generic_value_param` | Module-level generic value parameter (`parseGlobalGenericValueParamDecl`) |
| `extension`, `__extension` | Type extension (`parseExtensionDecl`) |
| `interface` | Interface (`parseInterfaceDecl`) |
| `__init` | Constructor (`parseConstructorDecl`) |
| `__subscript` | Subscript (`parseSubscriptDecl`) |
| `property` | Property (`parsePropertyDecl`) |
| `semantic` | HLSL-style semantic decl (`parseSemanticDecl`) |
| `cbuffer` | HLSL constant-buffer decl (`parseHLSLCBufferDecl`) |
| `tbuffer` | HLSL texture-buffer decl (`parseHLSLTBufferDecl`) |
| `syntax` | User-defined syntax (`parseSyntaxDecl`) |
| `attribute_syntax` | Attribute syntax (`parseAttributeSyntaxDecl`) |
| `import`, `__import` | Module import (`parseImportDecl`) |
| `__include` | Include directive (`parseIncludeDecl`) |
| `module` | Module declaration (`parseModuleDeclarationDecl`) |
| `implementing` | Module implementation declaration (`parseImplementingDecl`) |
| `let` | Immutable binding (`parseLetDecl`) |
| `var` | Mutable binding (`parseVarDecl`) |
| `func` | Function declaration (`parseFuncDecl`) |
| `namespace` | Namespace block (`parseNamespaceDecl`) |
| `using` | Using directive (`parseUsingDecl`) |
| `__ignored_block` | Compiler-internal ignored block |
| `__transparent_block` | Compiler-internal transparent block |
| `__file_decl` | Compiler-internal per-file decl group |
| `__require_capability` | Capability requirement (`parseRequireCapabilityDecl`) |

`struct`, `class`, `enum` themselves are also keywords but are
registered through the AST class-hierarchy registration done by
`populateBaseLanguageModule` rather than `_makeParseDecl`. See
[slang-syntax.cpp](../../../source/slang/slang-syntax.cpp).

## Modifier keywords

Registered through `_makeParseModifier` in
[slang-parser.cpp](../../../source/slang/slang-parser.cpp). Some are
"simple" (single keyword, single AST node class), others take
arguments (e.g. `layout`, `__target_intrinsic`).

### Simple modifiers

| Keyword | AST node |
| --- | --- |
| `in` | `InModifier` |
| `out` | `OutModifier` |
| `inout` | `InOutModifier` |
| `__ref` | `RefModifier` |
| `__constref` | `BorrowModifier` |
| `const` | `ConstModifier` |
| `__builtin` | `BuiltinModifier` |
| `highp`, `lowp`, `mediump` | `GLSLPrecisionModifier` |
| `__global` | `ActualGlobalModifier` |
| `inline` | `InlineModifier` |
| `public`, `private`, `internal` | `PublicModifier`, `PrivateModifier`, `InternalModifier` |
| `require` | `RequireModifier` |
| `param` | `ParamModifier` |
| `extern` | `ExternModifier` |
| `dyn` | `DynModifier` |
| `row_major`, `column_major` | `HLSLRowMajorLayoutModifier`, `HLSLColumnMajorLayoutModifier` |
| `nointerpolation`, `noperspective`, `linear`, `sample`, `centroid`, `precise` | Interpolation modifiers |
| `groupshared` | `HLSLGroupSharedModifier` |
| `static` | `HLSLStaticModifier` |
| `uniform` | `HLSLUniformModifier` |
| `export` | `HLSLExportModifier` |
| `dynamic_uniform` | `DynamicUniformModifier` |
| `override` | `OverrideModifier` |
| `point`, `line`, `triangle`, `lineadj`, `triangleadj` | Geometry-shader input modifiers |
| `vertices`, `indices`, `primitives`, `payload` | Mesh-shader output modifiers |
| `__prefix`, `__postfix` | Unary-operator placement modifiers |
| `__exported` | Re-export `import` modifier |

### Complex modifiers (take arguments)

| Keyword | Parses |
| --- | --- |
| `shared` | `parseSharedModifier` (sets HLSL groupshared / shared on context) |
| `volatile` | `parseVolatileModifier` |
| `coherent` | `parseCoherentModifier` |
| `restrict` | `parseRestrictModifier` |
| `readonly` | `parseReadonlyModifier` |
| `writeonly` | `parseWriteonlyModifier` |
| `layout` | `parseLayoutModifier` (GLSL-style layout block) |
| `hitAttributeEXT` | `parseHitAttributeEXTModifier` (raytracing) |
| `__intrinsic_op` | `parseIntrinsicOpModifier` |
| `__target_intrinsic` | `parseTargetIntrinsicModifier` |
| `__specialized_for_target` | `parseSpecializedForTargetModifier` |
| `__glsl_extension` | `parseGLSLExtensionModifier` |
| `__glsl_version` | `parseGLSLVersionModifier` |
| `__spirv_version` | `parseSPIRVVersionModifier` |
| `__wgsl_extension` | `parseWGSLExtensionModifier` |
| `__cuda_sm_version` | `parseCUDASMVersionModifier` |
| `__builtin_type` | `parseBuiltinTypeModifier` |
| `__builtin_requirement` | `parseBuiltinRequirementModifier` |
| `__magic_type` | `parseMagicTypeModifier` |
| `__magic_enum` | `parseMagicEnumModifier` |
| `__intrinsic_type` | `parseIntrinsicTypeModifier` |
| `__implicit_conversion` | `parseImplicitConversionModifier` |
| `__attributeTarget` | `parseAttributeTargetModifier` |

## Expression keywords

Registered through `_makeParseExpr` in
[slang-parser.cpp](../../../source/slang/slang-parser.cpp).

| Keyword | Parses |
| --- | --- |
| `this` | Self-reference (`parseThisExpr`) |
| `true`, `false` | Boolean literals |
| `nullptr` | Null pointer literal |
| `none` | `Optional`'s none literal |
| `try` | Error-handling expression (`parseTryExpr`) |
| `no_diff` | Non-differentiable wrapper (`parseTreatAsDifferentiableExpr`) |
| `__fwd_diff`, `fwd_diff` | Forward-mode differentiation (`parseForwardDifferentiate`) |
| `__bwd_diff`, `bwd_diff` | Reverse-mode differentiation (`parseBackwardDifferentiate`) |
| `__return_val` | Compiler-internal return-value reference |
| `__func_as_type` | Function-as-type reflection |
| `__dispatch_kernel` | Kernel-dispatch primitive |
| `sizeof`, `alignof`, `countof` | Size / alignment / element-count queries |
| `__first`, `__last`, `__trimFirst`, `__trimLast`, `__shapeConcat`, `__shapePermute`, `__shapeSwap`, `__shapeReduce`, `__packBranch` | Shape / pack utility expressions |
| `__getAddress` | Compiler-internal address-of |
| `__floatAsInt` | Compiler-internal bit reinterpretation |

## Core-module-supplied vocabulary

The four `*.meta.slang` files in
[source/slang/](../../../source/slang/) contribute additional names
to the default environment. They are not "keywords" in the parser-
syntax-table sense, but the parser does consult the environment to
classify identifiers, so from the user's perspective these names
behave like keywords. Process notes:

- [core.meta.slang](../../../source/slang/core.meta.slang) declares
  the built-in scalar / vector / matrix types, the `Optional`,
  `Result`, `Tuple` types, ranges, iterators, and core intrinsics.
- [hlsl.meta.slang](../../../source/slang/hlsl.meta.slang) layers in
  HLSL-compatibility names (`Texture2D`, `RWTexture2D`,
  `StructuredBuffer`, intrinsics like `mul`, `dot`, `length`, ...).
- [glsl.meta.slang](../../../source/slang/glsl.meta.slang) provides
  GLSL-flavored names (`vec3`, `mat4`, `gl_Position`, ...).
- [diff.meta.slang](../../../source/slang/diff.meta.slang)
  contributes the differentiable-pair types and helpers used by the
  autodiff machinery (see
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)).

The core-module pipeline as a whole is described in
[../cross-cutting/core-module.md](../cross-cutting/core-module.md).

## Reserved identifier prefixes

By convention:

- Names beginning with `__` (e.g. `__intrinsic_op`,
  `__target_intrinsic`, `__init`, `__subscript`, `__import`,
  `__include`, `__file_decl`) denote compiler-internal vocabulary that
  user code should not rely on. Many have public spellings without
  the underscores (`extension`, `import`, `init`, `subscript`,
  `include`).
- Names beginning with `gl_` come from the GLSL meta-module and stand
  for shader-stage built-ins.
- Names beginning with `SV_` (HLSL system-value semantics) appear as
  semantic strings rather than keywords; they are recognized during
  semantic checking
  ([../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)).

The forbidden / reserved set is not enforced lexically; it is policy
encoded by the meta-modules.

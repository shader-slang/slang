---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:08:24Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 16f77d4f6aaedcbfaa66fc7d04979d22e0ea47e46b6259a533f617fa9ec29f68
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Keywords and Built-in Syntax

This document inventories Slang's syntactic keywords for a developer
adding a new keyword or trying to understand why a specific identifier
is special. The non-obvious
fact it must convey: most "keywords" are not lexer-level reserved
words. They arrive at the parser as `TokenType::Identifier` (see
[tokens.md](tokens.md)) and become keywords only because a
`SyntaxDecl` in the active environment binds them to a parsing
callback. Adding or renaming a keyword therefore touches the parser's
syntax table or the core-module sources, not the lexer.

## Where keywords come from

Three sources contribute keywords:

1. **Hardcoded statement keywords** in
   [slang-parser.cpp](../../../../source/slang/slang-parser.cpp). The
   statement parser inspects identifiers via `LookAheadToken("if")`,
   `LookAheadToken("for")`, etc., and dispatches to dedicated parse
   functions.
2. **The parser's `SyntaxParseInfo` table**
   ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp), the
   array `g_parseSyntaxEntries[]`), which `getSyntaxParseInfos()`
   exposes as a view and `populateBaseLanguageModule()` registers into
   the default environment. This is the source of decl, modifier, and
   expression keywords.
3. **Core-module `*.meta.slang` declarations**. The meta-modules
   ([core.meta.slang](../../../../source/slang/core.meta.slang),
   [hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang),
   [glsl.meta.slang](../../../../source/slang/glsl.meta.slang),
   [diff.meta.slang](../../../../source/slang/diff.meta.slang)) declare
   built-in types, functions, and operators that contribute names to
   the default environment. These are processed at compiler startup,
   but they are built-in vocabulary rather than keywords: only a
   `SyntaxDecl` binding drives a parser callback, while a known type
   name participates in a few limited disambiguation decisions.

## Lexer-recognized keywords

The lexer does _not_ recognize alphabetic keywords. The only tokens
spelled out in the lexer / token catalog are punctuation and operators
— see [tokens.md](tokens.md) for the full list (`Semicolon`,
`Scope (::)`, `RightArrow (->)`, `DoubleRightArrow (=>)`, all the
`Op*` operators, etc.).

## Parser-registered syntax keywords

The bulk of Slang's keywords are recognized by the parser rather than
the lexer. Statement keywords are matched directly by the statement
parser; declaration, modifier, and expression keywords are registered
in the parser's syntax-decl table (`g_parseSyntaxEntries[]`) so they
can be redefined or extended through `syntax` / `attribute_syntax`
declarations.

Those two declarations are written
`syntax <name> [: <SyntaxClass>] [= <existingKeyword>];` and
`attribute_syntax [<Name>(<param> : <Type>, ...)] : <SyntaxClass>;`.
Both name an AST node class, which `parseSyntaxDecl` and
`parseAttributeSyntaxDecl` resolve through
`ASTBuilder::findSyntaxClass`, so neither form can introduce syntax
that builds a node the compiler does not already declare — the
extension point is a new spelling for an existing node, not a new
node. That is why both forms appear only in the core module in
practice; `syntax constexpr : ConstExprModifier;` and
`attribute_syntax [ExperimentalModule] : ExperimentalModuleAttribute;`
in [core.meta.slang](../../../../source/slang/core.meta.slang) are
representative. The `= <existingKeyword>` alias form copies the named
keyword's parse callback and syntax class so the new spelling is a
drop-in alias; no `.slang` source in the tree uses it at
`source_commit`.

### Statement keywords

Recognized in the statement parser by direct identifier comparison.
Cited line numbers refer to
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp) at
`source_commit`.

| Keyword               | Where parsed                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `if`                  | line 6921 (`LookAheadToken("if")`) in `Parser::ParseStatement` (line 6914). A second lookahead for `let` two tokens ahead (line 6923) routes the `if let` binding form to `parseIfLetStatement` (line 7284) instead of `parseIfStatement` (line 7373); `else` is consumed inside the latter at line 7382                                                                                                                                                                           |
| `for`                 | line 6932 (statement entry). The compile-time form is reached from `parseCompileTimeStmt` (line 6900), which reads a `$` and then checks for `for` at line 6903 before calling `parseCompileTimeForStmt` (line 6854). Its header is not the ordinary `(init; cond; update)` triple but `$for(i in Range(N))`: `parseCompileTimeForStmt` reads the loop variable, then the literal tokens `in` and `Range`, then one or two range expressions — `Range(end)` or `Range(begin, end)` |
| `while`               | line 6934                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `do`                  | line 6936                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `break`               | line 6938                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `continue`            | line 6940                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `return`              | line 6942                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `switch`              | line 6951                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `__target_switch`     | line 6953 (`parseTargetSwitchStmt`); compiler-internal                                                                                                                                                                                                                                                                                                                                                                                                                             |
| `__stage_switch`      | line 6955 (`parseStageSwitchStmt`); compiler-internal                                                                                                                                                                                                                                                                                                                                                                                                                              |
| `__intrinsic_asm`     | line 6957 (`parseIntrinsicAsmStmt`); compiler-internal                                                                                                                                                                                                                                                                                                                                                                                                                             |
| `case`                | line 6959 (and in the switch body at lines 6619, 6649)                                                                                                                                                                                                                                                                                                                                                                                                                             |
| `default`             | line 6961 (and in the switch body at lines 6625, 6649)                                                                                                                                                                                                                                                                                                                                                                                                                             |
| `__GPU_FOREACH`       | line 6963 (`ParseGpuForeachStmt`); compiler-internal                                                                                                                                                                                                                                                                                                                                                                                                                               |
| `discard`             | line 6944                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `defer`               | line 6969                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `throw`               | line 6977                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| `__requireCapability` | line 6981 (`Parser::ParseRequireCapabilityStatement`, line 7601); compiler-internal                                                                                                                                                                                                                                                                                                                                                                                                |
| `catch`               | `Parser::ParseDoCatchStatement` (line 7482), reached from `ParseDoStatement` at line 7527. `catch` does **not** pair with `try` at statement level                                                                                                                                                                                                                                                                                                                                 |

These keywords are not in the syntax-decl table because Slang treats
control-flow as a closed grammar; they cannot be redefined by user
code. Note that `try` is an _expression_ keyword (see
`## Expression keywords` below); the statement-level exception
handler is `do { ... } catch ( ... ) { ... }`, parsed at
[slang-parser.cpp lines
7482-7513](../../../../source/slang/slang-parser.cpp). The error
parameter is optional — a bare `catch` catches every error type — and
`ParseDoCatchStatement` loops while another `catch` follows, using each
`CatchStmt` as the `tryBody` of the next so a chain of `catch` clauses
nests rather than forming a flat list.

Each compiler-internal row parses a fixed form, so a reader who meets
one in core-module source can recognize it:

- `__stage_switch { case <stage>: ... default: ... }` shares
  `parseTargetSwitchStmtImpl` with `__target_switch`; the labels after
  `case` are capability names resolved by `findCapabilityName`, and an
  unrecognized one is diagnosed rather than treated as an identifier.
- `__intrinsic_asm "<text>";`, optionally followed by a
  comma-separated argument list before the semicolon
  (`__intrinsic_asm "(gl_SubgroupID)";`).
- `__GPU_FOREACH(<device>, <gridDims>, LAMBDA(uint3 <id>) { ... });`
  — the literal token `LAMBDA` is part of the grammar, not a
  user-supplied name.
- `__requireCapability(<capability>, ...);`, a comma-separated list of
  capability names, each again resolved by `findCapabilityName`.

### Decl keywords

Registered in `g_parseSyntaxEntries[]` at
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 10700
through `_makeParseDecl(...)` (defined at line 10671). Identifiers that begin with double
underscore (`__`) are intentionally namespaced as compiler-internal /
non-stable.

| Keyword                    | Parses                                                                                                                          |
| -------------------------- | ------------------------------------------------------------------------------------------------------------------------------- |
| `typedef`                  | C-style type alias (`parseTypeDef`)                                                                                             |
| `typealias`                | Slang-style type alias (`parseTypeAliasDecl`)                                                                                   |
| `associatedtype`           | Interface associated type (`parseAssocType`, line 4293)                                                                         |
| `__constraint`             | Interface-level constraint requirement (`parseInterfaceConstraintDecl`, line 4335)                                              |
| `__associatedfunc`         | Interface associated function (`parseAssocFunc`)                                                                                |
| `type_param`               | Module-level generic type parameter (`parseGlobalGenericTypeParamDecl`)                                                         |
| `__generic`                | Generic-parameter list head (`parseGenericDecl`)                                                                                |
| `__generic_value_param`    | Module-level generic value parameter (`parseGlobalGenericValueParamDecl`)                                                       |
| `extension`, `__extension` | Type extension (`parseExtensionDecl`)                                                                                           |
| `__func_extension`         | Function extension shorthand for custom derivatives (`parseFuncExtensionDecl`); experimental (gated by `-experimental-feature`) |
| `interface`                | Interface (`parseInterfaceDecl`)                                                                                                |
| `__init`                   | Constructor (`parseConstructorDecl`)                                                                                            |
| `__subscript`              | Subscript (`parseSubscriptDecl`)                                                                                                |
| `property`                 | Property (`parsePropertyDecl`)                                                                                                  |
| `semantic`                 | HLSL-style semantic decl (`parseSemanticDecl`)                                                                                  |
| `cbuffer`                  | HLSL constant-buffer decl (`parseHLSLCBufferDecl`)                                                                              |
| `tbuffer`                  | HLSL texture-buffer decl (`parseHLSLTBufferDecl`)                                                                               |
| `syntax`                   | User-defined syntax (`parseSyntaxDecl`)                                                                                         |
| `attribute_syntax`         | Attribute syntax (`parseAttributeSyntaxDecl`)                                                                                   |
| `import`, `__import`       | Module import (`parseImportDecl`)                                                                                               |
| `__include`                | Include directive (`parseIncludeDecl`)                                                                                          |
| `module`                   | Module declaration (`parseModuleDeclarationDecl`)                                                                               |
| `implementing`             | Module implementation declaration (`parseImplementingDecl`)                                                                     |
| `let`                      | Immutable binding (`parseLetDecl`)                                                                                              |
| `var`                      | Mutable binding (`parseVarDecl`)                                                                                                |
| `func`                     | Function declaration (`parseFuncDecl`)                                                                                          |
| `namespace`                | Namespace block (`parseNamespaceDecl`)                                                                                          |
| `using`                    | Using directive (`parseUsingDecl`)                                                                                              |
| `__ignored_block`          | Compiler-internal ignored block                                                                                                 |
| `__transparent_block`      | Compiler-internal transparent block                                                                                             |
| `__file_decl`              | Compiler-internal per-file decl group                                                                                           |
| `__require_capability`     | Capability requirement (`parseRequireCapabilityDecl`)                                                                           |

Three of the `__` rows have a form worth spelling out, because the
callback name alone does not imply one:

- `__constraint <type> == <type>;` states a type-equality requirement
  and `__constraint <type> : <type>;` a subtype requirement. Either
  becomes a `GenericTypeConstraintDecl` member of the enclosing
  interface, refining `This` or an associated type inherited from a
  base interface — `interface IDerived : IBase { __constraint DataType == This; }`
  requires `This.DataType == This` of every conformer. It is only
  meaningful in an interface body.
- `__associatedfunc <function-type> <name>;` — a type expression
  followed by the requirement's name. The core module uses it for the
  autodiff function requirements, e.g.
  `static __associatedfunc FwdDiffFuncType<FType> fwd_diff;` in
  [core.meta.slang](../../../../source/slang/core.meta.slang).
- `__func_extension` takes a higher-order target expression, a
  parameter list, an optional `throws` clause, an optional
  `-> <result type>`, and a body. The target is parsed by syntax-decl
  lookup rather than a hardcoded operator list, so it is written with
  one of the differentiation expression keywords, as in
  `__func_extension<T : __BuiltinFloatingPointType, let N : int> fwd_diff(CoopVec<T, N>::__subscript::get)(DifferentialPair<CoopVec<T, N>> self, int index) -> DifferentialPair<T> { ... }`
  from [hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang).

`struct`, `class`, and `enum` are also decl keywords, but they are
**not** registered through `g_parseSyntaxEntries[]` /
`_makeParseDecl`. Instead the parser dispatches on them via direct
identifier lookahead in the type-specifier parser
([slang-parser.cpp lines
3425-3445](../../../../source/slang/slang-parser.cpp)), reached from
`ParseDeclWithModifiers`
([slang-parser.cpp line
5805](../../../../source/slang/slang-parser.cpp)). The dedicated
parse routines (`ParseStruct` at line 6362, `ParseClass`
at line 6433, `parseEnumDecl` at line 6482) construct the
corresponding AST nodes directly.

At the parse level `class` is by far the narrower of `class` and
`struct`. `ParseClass` reads a required name, an optional inheritance
clause, and a body, and nothing else. `ParseStruct` additionally
accepts an anonymous form (it synthesizes a name when no identifier
follows), a generic parameter list through `parseOptGenericDecl`, a
body-less forward declaration `struct S;`, and the alias form
`struct S = T;` — so a generic aggregate has to be written as a
`struct`, because no path in `ParseClass` consumes a generic parameter
list. Beyond that the parser draws no distinction: it builds a
`ClassDecl` rather than a `StructDecl` and leaves every semantic
consequence of that choice to later stages
([../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)).

The same type-specifier parser also recognizes the variadic-pack type
forms `expand` and `each` by direct identifier lookahead
([slang-parser.cpp lines
3446-3455](../../../../source/slang/slang-parser.cpp)), alongside the
`__first` / `__last` / `__trimFirst` / `__trimLast` / `__shapeConcat`
/ `__shapePermute` / `__shapeSwap` / `__shapeReduce` / `__packBranch`
shape utilities (listed under `## Expression keywords`); none of these
are in `g_parseSyntaxEntries[]` either. Immediately after them the same
chain accepts `functype` (line 3462), which hands off to
`parseFuncTypeExpr` (line 3247) to parse a function type; it too is
matched by lookahead rather than registered as syntax. The spelling is
`functype(<parameter types>) -> <result type>` — zero or more
comma-separated parameter type expressions, a mandatory `->`, and a
result type. The core module uses it for higher-order parameters, as in
`This MapElement(functype(uint32_t, uint32_t, T) -> T mapOp)` in
[hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang); a
parameter of such a type accepts a function name as its argument. A
generic parameter list accepts the same keyword in the form
`functype F` to declare a function-typed generic parameter.

### Modifier keywords

Registered through `_makeParseModifier` in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp). Some are
"simple" (single keyword, single AST node class), others take
arguments (e.g. `layout`, `__target_intrinsic`).

#### Simple modifiers

| Keyword                                                                       | AST node                                                      |
| ----------------------------------------------------------------------------- | ------------------------------------------------------------- |
| `in`                                                                          | `InModifier`                                                  |
| `out`                                                                         | `OutModifier`                                                 |
| `inout`                                                                       | `InOutModifier`                                               |
| `__ref`                                                                       | `RefModifier`                                                 |
| `__constref`                                                                  | `BorrowModifier`                                              |
| `const`                                                                       | `ConstModifier`                                               |
| `__builtin`                                                                   | `BuiltinModifier`                                             |
| `highp`, `lowp`, `mediump`                                                    | `GLSLPrecisionModifier`                                       |
| `__global`                                                                    | `ActualGlobalModifier`                                        |
| `inline`                                                                      | `InlineModifier`                                              |
| `public`, `private`, `internal`                                               | `PublicModifier`, `PrivateModifier`, `InternalModifier`       |
| `require`                                                                     | `RequireModifier`                                             |
| `param`                                                                       | `ParamModifier`                                               |
| `extern`                                                                      | `ExternModifier`                                              |
| `dyn`                                                                         | `DynModifier`                                                 |
| `row_major`, `column_major`                                                   | `HLSLRowMajorLayoutModifier`, `HLSLColumnMajorLayoutModifier` |
| `nointerpolation`, `noperspective`, `linear`, `sample`, `centroid`, `precise` | Interpolation modifiers                                       |
| `groupshared`                                                                 | `HLSLGroupSharedModifier`                                     |
| `static`                                                                      | `HLSLStaticModifier`                                          |
| `uniform`                                                                     | `HLSLUniformModifier`                                         |
| `export`                                                                      | `HLSLExportModifier`                                          |
| `dynamic_uniform`                                                             | `DynamicUniformModifier`                                      |
| `override`                                                                    | `OverrideModifier`                                            |
| `point`, `line`, `triangle`, `lineadj`, `triangleadj`                         | Geometry-shader input modifiers                               |
| `vertices`, `indices`, `primitives`, `payload`                                | Mesh-shader output modifiers                                  |
| `__prefix`, `__postfix`                                                       | Unary-operator placement modifiers                            |
| `__exported`                                                                  | Re-export `import` modifier                                   |

Every row above is registered with the `_makeParseModifier(keyword,
getSyntaxClass<...>())` overload, whose callback is
`parseSimpleSyntax`: it constructs an instance of the class and reads
no further tokens. The keyword's entire parse-time meaning is therefore
the node in the second column, and nothing about the declaration it is
attached to is checked here. What each node class goes on to mean is
documented per node class in
[../ast-reference/modifiers.md](../ast-reference/modifiers.md) — that
page, not this one, is where a reader looks for the user-observable
effect of a modifier such as `row_major` or `nointerpolation`.

#### Callback-parsed modifiers (some take arguments)

| Keyword                    | Parses                                                            |
| -------------------------- | ----------------------------------------------------------------- |
| `shared`                   | `parseSharedModifier` (sets HLSL groupshared / shared on context) |
| `volatile`                 | `parseVolatileModifier`                                           |
| `coherent`                 | `parseCoherentModifier`                                           |
| `restrict`                 | `parseRestrictModifier`                                           |
| `readonly`                 | `parseReadonlyModifier`                                           |
| `writeonly`                | `parseWriteonlyModifier`                                          |
| `layout`                   | `parseLayoutModifier` (GLSL-style layout block)                   |
| `hitAttributeEXT`          | `parseHitAttributeEXTModifier` (raytracing)                       |
| `__intrinsic_op`           | `parseIntrinsicOpModifier`                                        |
| `__target_intrinsic`       | `parseTargetIntrinsicModifier`                                    |
| `__specialized_for_target` | `parseSpecializedForTargetModifier`                               |
| `__glsl_extension`         | `parseGLSLExtensionModifier`                                      |
| `__glsl_version`           | `parseGLSLVersionModifier`                                        |
| `__spirv_version`          | `parseSPIRVVersionModifier`                                       |
| `__wgsl_extension`         | `parseWGSLExtensionModifier`                                      |
| `__cuda_sm_version`        | `parseCUDASMVersionModifier`                                      |
| `__builtin_type`           | `parseBuiltinTypeModifier`                                        |
| `__builtin_requirement`    | `parseBuiltinRequirementModifier`                                 |
| `__magic_type`             | `parseMagicTypeModifier`                                          |
| `__magic_enum`             | `parseMagicEnumModifier`                                          |
| `__intrinsic_type`         | `parseIntrinsicTypeModifier`                                      |
| `__implicit_conversion`    | `parseImplicitConversionModifier`                                 |
| `__attributeTarget`        | `parseAttributeTargetModifier`                                    |

Which of these actually take arguments is decided by the callback, not
by the heading. Six of them read no tokens at all: `shared`,
`volatile`, `coherent`, `restrict`, `readonly`, and `writeonly` are
written bare, and their callbacks exist to choose or duplicate nodes
rather than to parse operands — `shared` builds
`HLSLGroupSharedModifier` when the parser's `allowGLSLInput` option is
set and `HLSLEffectSharedModifier` otherwise, and `volatile` builds
both the HLSL and the GLSL node while diagnosing the keyword as
deprecated from language version 2025 and removed from 2026.
`hitAttributeEXT` likewise takes no arguments.

`layout` takes a parenthesized, comma-separated list of GLSL
qualifiers, each a bare name or `name = expr`, as in
`layout(local_size_x = 8, std430)`. The remaining, `__`-prefixed rows
take a parenthesized argument list: an identifier for
`__glsl_extension(GL_KHR_shader_subgroup_basic)`,
`__wgsl_extension(subgroups)`, `__specialized_for_target(glsl)`, and
`__attributeTarget(<SyntaxClass>)`; an integer for
`__glsl_version(430)`, `__builtin_type(<tag>)`, and
`__builtin_requirement(<kind>)`; a `major.minor` or quoted version for
`__spirv_version(1.3)` and `__cuda_sm_version(7.0)`; a name with an
optional tag for `__magic_type(<Name>[, <tag>])` and `__magic_enum`; an
IR opcode plus optional integer operands for
`__intrinsic_type(<op>[, <operand>]...)`; and a target name plus an
optional definition for `__target_intrinsic(hlsl, "...")`. Four of them
accept the parentheses only optionally, and mean something without
them: bare `__intrinsic_op` derives the opcode from the function name
instead of taking the integer or identifier it otherwise accepts, and
`__target_intrinsic`, `__specialized_for_target`, and
`__implicit_conversion` fall back to their defaults.

### Expression keywords

Registered through `_makeParseExpr` in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp).

| Keyword                                                                                                                             | Parses                                                                                                                                                                                                                                                         |
| ----------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `this`                                                                                                                              | Self-reference (`parseThisExpr`)                                                                                                                                                                                                                               |
| `true`, `false`                                                                                                                     | Boolean literals                                                                                                                                                                                                                                               |
| `nullptr`                                                                                                                           | Null pointer literal                                                                                                                                                                                                                                           |
| `none`                                                                                                                              | `Optional`'s none literal                                                                                                                                                                                                                                      |
| `try`                                                                                                                               | Error-handling expression (`parseTryExpr`)                                                                                                                                                                                                                     |
| `no_diff`                                                                                                                           | Non-differentiable wrapper (`parseTreatAsDifferentiableExpr`)                                                                                                                                                                                                  |
| `__fwd_diff`, `fwd_diff`                                                                                                            | Forward-mode differentiation (`parseForwardDifferentiate`)                                                                                                                                                                                                     |
| `__bwd_diff`, `bwd_diff`                                                                                                            | Reverse-mode differentiation (`parseBackwardDifferentiate`)                                                                                                                                                                                                    |
| `__apply`                                                                                                                           | Apply-for-backward higher-order expression (`parseApplyForBwd`); used inside `__func_extension` to expose the primal-with-context companion to a custom `bwd_diff`; experimental                                                                               |
| `new`                                                                                                                               | Heap-style allocation expression; parsed specially by the `AdvanceIf(parser, "new")` branch of `parsePrefixExpr` at [slang-parser.cpp line 9686](../../../../source/slang/slang-parser.cpp) (`parsePrefixExpr` defined at line 9678; not via `_makeParseExpr`) |
| `__return_val`                                                                                                                      | Compiler-internal return-value reference                                                                                                                                                                                                                       |
| `__func_as_type`                                                                                                                    | Function-as-type reflection                                                                                                                                                                                                                                    |
| `__dispatch_kernel`                                                                                                                 | Kernel-dispatch primitive                                                                                                                                                                                                                                      |
| `sizeof`, `alignof`, `countof`                                                                                                      | Size / alignment / element-count queries                                                                                                                                                                                                                       |
| `__first`, `__last`, `__trimFirst`, `__trimLast`, `__shapeConcat`, `__shapePermute`, `__shapeSwap`, `__shapeReduce`, `__packBranch` | Shape / pack utility expressions                                                                                                                                                                                                                               |
| `__getAddress`                                                                                                                      | Compiler-internal address-of                                                                                                                                                                                                                                   |
| `__floatAsInt`                                                                                                                      | Compiler-internal bit reinterpretation                                                                                                                                                                                                                         |

The operand shape of each row is fixed by its callback, and most but
not all of the rows are parenthesized:

- `try` and `no_diff` take a following leaf expression with no
  parentheses of their own — `try f(x)`, `no_diff f(x)`.
- `__return_val` takes no operand at all; it is a bare reference to the
  pending return value.
- `fwd_diff` / `__fwd_diff`, `bwd_diff` / `__bwd_diff`, `__apply`,
  `__func_as_type`, `__getAddress`, `__floatAsInt`, and `countof` take
  exactly one parenthesized operand — `fwd_diff(f)`, `countof(pack)`.
- `sizeof` and `alignof` take one operand plus an optional second
  data-layout operand: `sizeof(T)` or `sizeof(T, Std140DataLayout)`.
- `__first`, `__last`, `__trimFirst`, and `__trimLast` take one pack
  operand; `__shapePermute` and `__shapeReduce` take two;
  `__shapeConcat`, `__shapeSwap`, and `__packBranch` take three
  (`__packBranch(<pack>, <empty type>, <non-empty type>)`).
- `__dispatch_kernel(<function>, <dispatch size>, <thread-group size>)`
  takes three.
- `new` is a prefix operator over a postfix expression, so it is
  written `new T(args)` — the type name and its argument list are
  parsed first and then folded into the `NewExpr`.

## Core-module syntax declarations

The four `*.meta.slang` files in
[source/slang/](../../../../source/slang) contribute additional names
to the default environment. They are not "keywords" in the parser-
syntax-table sense: the parser's syntax lookup accepts only a
`SyntaxDecl`, so these names are built-in vocabulary that the parser
consults for limited type-name disambiguation, not parsing keywords.
Process notes:

- [core.meta.slang](../../../../source/slang/core.meta.slang) declares
  the built-in scalar / vector / matrix types, the `Optional` and
  `Tuple` types, and core intrinsics.
- [hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang) layers in
  HLSL-compatibility names (`Texture2D`, `RWTexture2D`,
  `StructuredBuffer`, intrinsics like `mul`, `dot`, `length`, and the
  wave intrinsics). Two kinds of "wave builtin" live here and they are
  not interchangeable. `WaveGetWaveIndex()` is an ordinary core-module
  function: it carries
  `[require(cuda_glsl_hlsl_metal_spirv_wgsl, subgroup_workgroup_index)]`
  and a `__target_switch` body with an arm per target, so its
  availability is stated in the module itself. `SV_WaveIndex` and
  `SV_GroupIndex` are not functions but system-value semantics, written
  after a `:` on the hidden `in` globals `__builtinWaveIndex` and
  `__builtinGroupIndex` near the top of the file; the module attaches no
  capability requirement to them, and which targets accept one is
  decided by entry-point varying-parameter legalization
  ([../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)), not
  here. `WaveGetWaveIndex()` is therefore the portable spelling.
- The same file declares the descriptor-heap vocabulary —
  `UntypedResourceHandle` and `UntypedSamplerHandle`, together with the
  `__ResourceDescriptorHeapType` / `__SamplerDescriptorHeapType` types
  whose `__subscript` turns an index into a handle — each gated by
  `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]` so unsupported
  targets are diagnosed at the indexing site. The user-level spelling of
  those two types is the pair of `static const` globals
  `ResourceDescriptorHeap` and `SamplerDescriptorHeap`, so the surface
  form is `ResourceDescriptorHeap[i]`. The untyped handle it yields is
  not written down: every heap-castable resource type declares an
  `__implicit_conversion` constructor taking the handle, so the
  concrete type is recovered from the assignment target, as in
  `RWStructuredBuffer<uint> buf = ResourceDescriptorHeap[0];`.
- [glsl.meta.slang](../../../../source/slang/glsl.meta.slang) provides
  GLSL-flavored names (`vec3`, `mat4`, `gl_Position`, ...).
- [diff.meta.slang](../../../../source/slang/diff.meta.slang)
  contributes the differentiable-pair types and helpers used by the
  autodiff machinery (see
  [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)).

The core-module pipeline as a whole is described in
[../cross-cutting/core-module.md](../cross-cutting/core-module.md).

## Reserved identifier prefixes

By convention:

- Names beginning with `__` (e.g. `__intrinsic_op`,
  `__target_intrinsic`, `__init`, `__subscript`, `__import`,
  `__include`, `__constraint`, `__file_decl`) denote compiler-internal vocabulary that
  user code should not rely on. A few have parser-registered public
  spellings without the underscores (`extension`, `import`); `__init`,
  `__subscript`, and `__include` have no underscore-free spelling in
  `g_parseSyntaxEntries[]`.
- Names beginning with `gl_` come from the GLSL meta-module and stand
  for shader-stage built-ins. They are ordinary global declarations —
  `public out float4 gl_Position : SV_Position;` — not a lexical class.
- Names beginning with `SV_` (HLSL system-value semantics) appear as
  semantic strings rather than keywords; they are recognized during
  semantic checking
  ([../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)).

The forbidden / reserved set is not enforced lexically; it is policy
encoded by the meta-modules. Both prefixes are advisory: no diagnostic
stands behind either, so a user declaration whose name begins with one
is an ordinary declaration. The parser inspects the `gl_` prefix in
exactly one place — a GLSL interface block whose name starts with `gl_`
is taken as a redeclaration of a built-in block and replaced with an
`EmptyDecl` — and never inspects `SV_` at all. The parser's only
reserved-name check is `isReservedKeywordName`, which warns
(`KeywordUsedAsName`) when a declarator is named `struct`, `class`,
`enum`, `typealias`, or `typedef` — the five spellings that open a type
specifier, so a name that collides with one cannot be referenced at
statement head. Its comment states the rule the rest of the vocabulary
obeys: almost every keyword is contextual and may be shadowed by a
user-defined name, including declaration keywords such as `func`,
`let`, `var`, `interface`, `extension`, and `import`.

The `__` prefix is the one that is really load-bearing, and it is
load-bearing only for the exact spellings the tables above list. Only
the `__`-prefixed forms of `__init`, `__subscript`, and `__include` are
registered in `g_parseSyntaxEntries[]`; the bare `init`, `subscript`,
and `include` are ordinary identifiers, usable as function names and
callable at statement head.

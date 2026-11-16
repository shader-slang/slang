---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-03T00:00:00Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: 686b8730f14d503c2c40ba8ae76a5c904f438901aace8656656d58178d874fee
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

The lexer does *not* recognize alphabetic keywords. The only tokens
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
node. That is why the forms are used mainly in core-module
source; user source can also declare them, as
`tests/diagnostics/redeclaration-user-alias-modifier.slang` does. Their shapes, taken from the
grammar above, are `syntax <name> : <SyntaxClass>;` and
`attribute_syntax [<Name>] : <SyntaxClass>;`;
`syntax constexpr : ConstExprModifier;` and
`attribute_syntax [ExperimentalModule] : ExperimentalModuleAttribute;`
in [core.meta.slang](../../../../source/slang/core.meta.slang) are
representative. The `= <existingKeyword>` alias form copies the named
keyword's parse callback and user data, and inherits its syntax class
only when no explicit class was given (`slang-parser.cpp` 5522-5529),
so the new spelling is a drop-in alias in that case; no `.slang` source in the tree uses it at
`source_commit`.

### Statement keywords

Recognized in the statement parser by direct identifier comparison.
Cited line numbers refer to
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp) at
`source_commit`.

| Keyword | Where parsed |
| --- | --- |
| `if` | line 7288 (`LookAheadToken("if")`) in `Parser::ParseStatement` (line 7281, whose second parameter is the `AllowCaseDefaultStatements` flag described in the `case` row). A second lookahead for `let` two tokens ahead (line 7290) routes the `if let` binding form to `parseIfLetStatement` (line 7677) instead of `parseIfStatement` (line 7773); `else` is consumed inside the latter at line 7784 |
| `for` | line 7299 (statement entry). The compile-time form is reached from `parseCompileTimeStmt` (line 7267), which reads a `$` and then checks for `for` at line 7270 before calling `parseCompileTimeForStmt` (line 7210). Its header is not the ordinary `(init; cond; update)` triple but `$for(i in Range(N))`: `parseCompileTimeForStmt` reads the loop variable, then the literal tokens `in` and `Range`, then one or two range expressions — `Range(end)` or `Range(begin, end)`. It first checks the module's language version: from 2025 it reports `CompileTimeForIsDeprecated` and from 202c `CompileTimeForIsRemoved` (lines 7218-7227), and then parses the loop regardless |
| `while` | line 7301 |
| `do` | line 7303 |
| `break` | line 7305 |
| `continue` | line 7307 |
| `return` | line 7309 |
| `switch` | line 7318 |
| `__target_switch` | line 7320 (`parseTargetSwitchStmt`); compiler-internal |
| `__stage_switch` | line 7322 (`parseStageSwitchStmt`); compiler-internal |
| `__intrinsic_asm` | line 7324 (`parseIntrinsicAsmStmt`); compiler-internal |
| `case` | line 7326 (and in the switch body at lines 6975, 7005). The statement parser accepts `case` anywhere and then diagnoses it: `ParseCaseStmt` always builds a node, and a caller that did not pass `AllowCaseDefaultStatements::Allow` reports `CaseOutsideSwitch` (E39999) at line 7331 |
| `default` | line 7333 (and in the switch body at lines 6981, 7005). Same shape as `case`: parsed unconditionally, then reported as `DefaultOutsideSwitch` (E39999) at line 7338 unless the caller allowed it |
| `__GPU_FOREACH` | line 7340 (`ParseGpuForeachStmt`); compiler-internal |
| `discard` | line 7311 |
| `defer` | line 7346 |
| `throw` | line 7354 |
| `__requireCapability` | line 7358 (`Parser::ParseRequireCapabilityStatement`, line 8002); compiler-internal |
| `catch` | `Parser::ParseDoCatchStatement` (line 7882), reached from `ParseDoStatement` at line 7929. `catch` does **not** pair with `try` at statement level |

These keywords are not in the syntax-decl table because Slang treats
control-flow as a closed grammar; they cannot be redefined by user
code. Note that `try` is an *expression* keyword (see
`## Expression keywords` below), although `ParseStatement` also has a
`try` branch (line 7350) that hands a statement starting with `try`
to the expression-statement parser; the statement-level exception
handler is `do { ... } catch ( ... ) { ... }`, parsed at
[slang-parser.cpp lines
7882-7913](../../../../source/slang/slang-parser.cpp). The error
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
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 11106
through `_makeParseDecl(...)` (defined at line 11077). Identifiers that begin with double
underscore (`__`) are intentionally namespaced as compiler-internal /
non-stable.

| Keyword | Parses |
| --- | --- |
| `typedef` | C-style type alias (`parseTypeDef`) |
| `typealias` | Slang-style type alias (`parseTypeAliasDecl`) |
| `associatedtype` | Interface associated type (`parseAssocType`, line 4551) |
| `__constraint` | Interface-level constraint requirement (`parseInterfaceConstraintDecl`, line 4593) |
| `__associatedfunc` | Interface associated function (`parseAssocFunc`, line 4620) |
| `type_param` | Module-level generic type parameter (`parseGlobalGenericTypeParamDecl`) |
| `__generic` | Generic-parameter list head (`parseGenericDecl`) |
| `__generic_value_param` | Module-level generic value parameter (`parseGlobalGenericValueParamDecl`) |
| `extension`, `__extension` | Type extension (`parseExtensionDecl`) |
| `__func_extension` | Function extension shorthand for custom derivatives (`parseFuncExtensionDecl`, line 4494); experimental: semantic checking rejects it unless the `-experimental-feature` flag, which takes no value, is given |
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
  [core.meta.slang](../../../../source/slang/core.meta.slang) (line 771).
- `__func_extension` takes a higher-order target expression, a
  parameter list, an optional `throws` clause, an optional
  `-> <result type>`, and a body. The target is parsed by syntax-decl
  lookup rather than a hardcoded operator list, so it is written with
  one of the differentiation expression keywords, as in
  `__func_extension<T : __BuiltinFloatingPointType, let N : int> fwd_diff(CoopVec<T, N>::__subscript::get)(DifferentialPair<CoopVec<T, N>> self, int index) -> DifferentialPair<T> { ... }`
  from [hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang)
  (line 33606; the `-experimental-feature` flag is the whole
  invocation, as above).

`struct`, `class`, and `enum` are also decl keywords, but they are
**not** registered through `g_parseSyntaxEntries[]` /
`_makeParseDecl`. Instead the parser dispatches on them via direct
identifier lookahead in the type-specifier parser
`_parseSimpleTypeSpec`
([slang-parser.cpp lines
3662-3688](../../../../source/slang/slang-parser.cpp)), reached through
`ParseDeclaratorDecl` and `_parseTypeSpec` from the fallback case of
`ParseDeclWithModifiers`
([slang-parser.cpp line
6268](../../../../source/slang/slang-parser.cpp)). The dedicated
parse routines (`ParseStruct` at line 6719, `ParseClass`
at line 6789, `parseEnumDecl` at line 6838) construct the
corresponding AST nodes directly. A block statement also routes a
leading `struct`, or in HLSL source a leading `class`, to the
declaration parser (`isLookaheadADeclKeywordAllowedInStmtContexts`,
line 7507, used at line 7567).

What `class` means to a user depends on the source language the parser
was given (`Parser::getSourceLanguage()`). In HLSL source (lines 3669-3677)
`class` is parsed by `ParseStruct` with an `HLSLClassDecl` node, so it
takes the full struct grammar and its comment calls the result a type
with value semantics. In any other source language `ParseClass`
builds a `ClassDecl`, which the code comment describes as the
reference-type representation. `ParseClass` reads a required name, an
optional inheritance clause, and a body, and nothing else.
`ParseStruct` additionally accepts an anonymous form (it synthesizes a
name when no identifier follows), a generic parameter list through
`parseOptGenericDecl`, a body-less forward declaration `struct S;`, and
the alias form `struct S = T;` — so outside HLSL a generic aggregate has
to be written as a `struct`, because no path in `ParseClass` consumes a
generic parameter list. Beyond that the parser draws no distinction and
leaves every semantic consequence of the choice to later stages
([../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)).

The same type-specifier parser also recognizes the variadic-pack type
forms `expand` and `each` by direct identifier lookahead
([slang-parser.cpp lines
3689-3698](../../../../source/slang/slang-parser.cpp)), in the same
branch as the `__first` / `__last` / `__trimFirst` / `__trimLast` /
`__shapeConcat` / `__shapePermute` / `__shapeSwap` / `__shapeReduce` /
`__packBranch` shape utilities, which hands off to `parsePrefixExpr`.
`expand` and `each` are not in `g_parseSyntaxEntries[]`; the shape
utilities are (listed under `## Expression keywords`) and are reached
by both routes. After that branch the same chain accepts `functype`
(line 3705), which hands off to `parseFuncTypeExpr` (line 3403) to
parse a function type; it too is matched by lookahead rather than
registered as syntax. `__func_as_type` is matched the same way (line
3710). The spelling is
`functype(<parameter types>) -> <result type>` — zero or more
comma-separated parameter type expressions, a mandatory `->`, and a
result type. The core module uses it for higher-order parameters, as in
`This MapElement(functype(uint32_t, uint32_t, T)->T mapOp)` in
[hlsl.meta.slang](../../../../source/slang/hlsl.meta.slang) (line
30554); a parameter of such a type accepts a function name as its
argument. Nothing in the parser says whether a value of such a type can
be called, so this document does not claim it. A generic parameter list
accepts the same keyword in the form `functype F` (line 1731) to
declare a function-typed generic parameter.

When the source language is HLSL, the first branch of that chain
(line 3658) recognizes `unsigned` and `signed` through
`isHLSLTraditionalIntegerTypeSpecifierStart`: `unsigned int` is
accepted (bare `unsigned` too), while `signed` and the `char` /
`short` / `long` widths are diagnosed.

### Modifier keywords

Registered through `_makeParseModifier` in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp). Some are
"simple" (single keyword, single AST node class), others take
arguments (e.g. `layout`, `__target_intrinsic`).

#### Simple modifiers

| Keyword | AST node |
| --- | --- |
| `in` | `InModifier` |
| `out` | `OutModifier` |
| `inout` | `InOutModifier` |
| `__ref` | `RefModifier` |
| `__ref_readonly`, `__ref_writeonly` | `RefReadOnlyModifier`, `RefWriteOnlyModifier` |
| `__borrow`, `__constref` | `BorrowModifier` (both spellings) |
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

| Keyword | Parses | Written as |
| --- | --- | --- |
| `shared` | `parseSharedModifier` (line 10661) | `shared` (bare) |
| `volatile` | `parseVolatileModifier` (line 10679) | `volatile` (bare) |
| `coherent` | `parseCoherentModifier` | `coherent` (bare) |
| `restrict` | `parseRestrictModifier` | `restrict` (bare) |
| `readonly` | `parseReadonlyModifier` | `readonly` (bare) |
| `writeonly` | `parseWriteonlyModifier` | `writeonly` (bare) |
| `layout` | `parseLayoutModifier` (line 10764; GLSL-style layout block) | `layout(local_size_x = 8, std430)` |
| `hitAttributeEXT` | `parseHitAttributeEXTModifier` (raytracing) | `hitAttributeEXT` (bare) |
| `__intrinsic_op` | `parseIntrinsicOpModifier` | `__intrinsic_op(<opcode or name>)`, or bare |
| `__target_intrinsic` | `parseTargetIntrinsicModifier` | `__target_intrinsic(hlsl, "name")`, or bare |
| `__specialized_for_target` | `parseSpecializedForTargetModifier` | `__specialized_for_target(glsl)`, or bare |
| `__glsl_extension` | `parseGLSLExtensionModifier` | `__glsl_extension(GL_KHR_shader_subgroup_basic)` |
| `__glsl_version` | `parseGLSLVersionModifier` | `__glsl_version(430)` |
| `__spirv_version` | `parseSPIRVVersionModifier` | `__spirv_version(1.3)` |
| `__wgsl_extension` | `parseWGSLExtensionModifier` | `__wgsl_extension(subgroups)` |
| `__cuda_sm_version` | `parseCUDASMVersionModifier` | `__cuda_sm_version(7.0)` |
| `__builtin_type` | `parseBuiltinTypeModifier` | `__builtin_type(<integer tag>)` |
| `__builtin_requirement` | `parseBuiltinRequirementModifier` | `__builtin_requirement(<integer kind>)` |
| `__magic_type` | `parseMagicTypeModifier` | `__magic_type(<Name>[, <tag>])` |
| `__magic_enum` | `parseMagicEnumModifier` | `__magic_enum(<Name>[, <tag>])` |
| `__intrinsic_type` | `parseIntrinsicTypeModifier` | `__intrinsic_type(<IR op>[, <operand>]...)` |
| `__implicit_conversion` | `parseImplicitConversionModifier` | `__implicit_conversion(<cost>[, <kind>])`, or bare |
| `__attributeTarget` | `parseAttributeTargetModifier` | `__attributeTarget(<SyntaxClass>)` |

Which of these actually take arguments is decided by the callback, not
by the heading: the "Written as" column gives the form each callback
reads. None of the callbacks checks which declaration the modifier is
attached to; that is decided after parsing. Six rows read no tokens at
all, and their callbacks exist to choose or duplicate nodes rather than
to parse operands. `shared` builds `HLSLGroupSharedModifier` when the
parser's source language is GLSL and `HLSLEffectSharedModifier`
otherwise. `volatile` builds both the HLSL and the GLSL node; outside
GLSL source it also diagnoses the keyword as deprecated from language
version 2025 and removed from 2026. `hitAttributeEXT` likewise takes no
arguments.

`layout` takes a parenthesized, comma-separated list of GLSL
qualifiers, each a bare name or `name = expr`. The `__`-prefixed rows
other than the four optional-parenthesis ones take a parenthesized
argument list whose elements are an identifier, an integer, or, for
`__spirv_version` and `__cuda_sm_version`, a `major.minor` number or
quoted version string. For `__implicit_conversion` the cost is an
integer or `constraint` (optionally followed by `+`). The four
optional-parenthesis rows mean something without the parentheses: bare
`__intrinsic_op` derives the opcode from the function name instead of
taking the integer or identifier it otherwise accepts, and
`__target_intrinsic`, `__specialized_for_target`, and
`__implicit_conversion` fall back to their defaults.

### Expression keywords

Registered through `_makeParseExpr` in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp).

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
| `__apply` | Apply-for-backward higher-order expression (`parseApplyForBwd`); used inside `__func_extension` to expose the primal-with-context companion to a custom `bwd_diff`; experimental |
| `new` | Heap-style allocation expression; parsed specially by the `AdvanceIf(parser, "new")` branch of `parsePrefixExpr` at [slang-parser.cpp line 10093](../../../../source/slang/slang-parser.cpp) (`parsePrefixExpr` defined at line 10085; not via `_makeParseExpr`) |
| `__return_val` | Compiler-internal return-value reference |
| `__func_as_type` | Function-as-type reflection |
| `__dispatch_kernel` | Kernel-dispatch primitive |
| `sizeof`, `alignof`, `countof` | Size / alignment / element-count queries |
| `__first`, `__last`, `__trimFirst`, `__trimLast`, `__shapeConcat`, `__shapePermute`, `__shapeSwap`, `__shapeReduce`, `__packBranch` | Shape / pack utility expressions |
| `__getAddress` | Compiler-internal address-of |
| `__floatAsInt` | Compiler-internal bit reinterpretation |

The operand shape of each row is fixed by its callback, and most but
not all of the rows are parenthesized:

- `try` and `no_diff` take a following leaf expression with no
  parentheses of their own — `try f(x)`, `no_diff f(x)`.
- `__return_val` takes no operand at all; it is a bare reference to the
  pending return value.
- `fwd_diff` / `__fwd_diff`, `bwd_diff` / `__bwd_diff`, `__apply`,
  `__func_as_type`, `__getAddress`, `__floatAsInt`, and `countof` take
  exactly one parenthesized operand — `fwd_diff(f)`,
  `__func_as_type(f)`, `__getAddress(v)`, `__floatAsInt(x)`,
  `countof(pack)`.
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
  and a `__target_switch` body with an arm per target (line 18430), so
  its availability is stated in the module itself. `SV_WaveIndex` and
  `SV_GroupIndex` are not functions but system-value semantics, written
  after a `:` on the hidden `in` globals `__builtinWaveIndex` and
  `__builtinGroupIndex` near the top of the file (lines 15 and 17); the
  module attaches no capability requirement to them, and which targets
  accept one is decided by entry-point varying-parameter legalization
  ([slang-ir-legalize-varying-params.cpp](../../../../source/slang/slang-ir-legalize-varying-params.cpp)),
  not here. In that file only two per-target contexts map `SV_WaveIndex`:
  the Metal one (line 4608, to `simdgroup_index_in_threadgroup`) and the
  WGSL one (line 5380, to `subgroup_id`). It has no arm for it on any
  other target, so a source written with the semantic is portable to
  those two only; the GLSL / SPIR-V path is a separate legalizer that
  this file does not contain. `WaveGetWaveIndex()` is therefore the
  portable spelling.
- The same file declares the descriptor-heap vocabulary —
  `UntypedResourceHandle` and `UntypedSamplerHandle` (lines 29254 and
  29263), together with the `__ResourceDescriptorHeapType` /
  `__SamplerDescriptorHeapType` types (lines 29279 and 29288) whose
  `__subscript` turns an index into a handle — each gated by
  `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]` so unsupported
  targets are diagnosed at the indexing site. The user-level spelling of
  those two types is the pair of `static const` globals
  `ResourceDescriptorHeap` and `SamplerDescriptorHeap` (lines 29297 and
  29298), so the surface form is `ResourceDescriptorHeap[i]`. The
  untyped handle it yields is not written down: every heap-castable
  resource type declares an `__implicit_conversion` constructor taking
  the handle, so the concrete type is recovered from the assignment
  target, as in `RWStructuredBuffer<uint> buf = ResourceDescriptorHeap[0];`.
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
encoded by the meta-modules. Both prefixes are advisory naming policy:
the parser raises no diagnostic for either, so a user declaration
whose name begins with one, such as a local named `SV_userVar`, is
parsed like any other declaration. The parser inspects the `gl_` prefix
in exactly one place (line 6252) — a GLSL interface block whose name starts with `gl_`
is taken as a redeclaration of a built-in block and replaced with an
`EmptyDecl` — and never inspects `SV_` at all. The parser's only
reserved-name check is `isReservedKeywordName` (line 2607), which warns
(`KeywordUsedAsName`) when a declarator is named `struct`, `class`,
`enum`, `typealias`, or `typedef` — the five spellings that open a type
specifier, so a name that collides with one cannot be referenced at
statement head. Its comment states the rule the rest of the vocabulary
obeys: almost every keyword is contextual and may be shadowed by a
user-defined name, including declaration keywords such as `func`,
`let`, `var`, `interface`, `extension`, and `import`.

The one place the parser looks at a `__` prefix on a user-chosen name is
inline SPIR-V assembly: when parsing the core module, a named `%id`
register must start with `__` (an assertion at
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
9828-9831), so that the register reads as compiler-internal.

The `__` prefix is the one that is really load-bearing, and it is
load-bearing only for the exact spellings the tables above list. Only
the `__`-prefixed forms of `__init`, `__subscript`, and `__include` are
registered in `g_parseSyntaxEntries[]`; the bare `init`, `subscript`,
and `include` are ordinary identifiers, usable as function names and
callable at statement head.

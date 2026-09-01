---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:25:56Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: 0199faf4e426ba466aba526a350e702abad4289ecdf404224cde287299d5da24
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Parse and AST Construction

This document covers the parsing stage: turning the flat token list
produced by [01-lex-preprocess.md](01-lex-preprocess.md) into a
strongly-typed AST. The intended reader is a developer adding new
syntax, modifying an AST node, or debugging a parse error.

## Inputs and outputs

- **Input**: a `TokenSpan` plus a `TranslationUnitRequest`,
  `Scope*` for the outer environment, and a `ContainerDecl*` parent
  (the receiving namespace / module decl). Entry point declared in
  [slang-parser.h](../../../../source/slang/slang-parser.h):

  ```cpp
  void parseSourceFile(
      ASTBuilder* astBuilder,
      TranslationUnitRequest* translationUnit,
      SourceLanguage sourceLanguage,
      TokenSpan const& tokens,
      DiagnosticSink* sink,
      Scope* outerScope,
      ContainerDecl* parentDecl);
  ```

- **Output**: AST nodes attached to `parentDecl`, allocated through
  the `ASTBuilder` passed in. Function and method bodies are still
  unparsed at this stage — see "Two-stage parsing" below.

## Parser

Implemented in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp). The design
is a recursive-descent parser. Because tokens are pre-collected into a
flat list (see [01-lex-preprocess.md](01-lex-preprocess.md)), the
parser can use arbitrary lookahead, although it rarely looks ahead more
than one token.

The token stream is consumed through `TokenReader`
([slang-lexer.h](../../../../source/compiler-core/slang-lexer.h)), which
exposes `peekToken`, `peekTokenType`, `peekLoc`, `advanceToken`, plus
`ParsingCursor` save / restore for backtracking.

### Two-stage parsing

Slang parses in two stages:

1. **Decl-parse stage.** Top-level declarations are parsed normally,
   but when the parser reaches a `{ ... }` function body it does not
   recurse into it; instead, `parseOptBody`
   ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 2219) tracks brace nesting and copies the enclosed tokens into an
   `UnparsedStmt` AST node, which also records the scopes the body was
   written in (`currentScope`, `outerScope`).
2. **Body-parse stage.** When semantic checking encounters an
   `UnparsedStmt`, it spawns a new `Parser` initialized in body mode
   and re-parses those tokens with a back-pointer to the
   `SemanticsVisitor`. The auxiliary entry point declared in
   [slang-parser.h](../../../../source/slang/slang-parser.h) is:

   ```cpp
   Stmt* parseUnparsedStmt(
       ASTBuilder* astBuilder,
       SemanticsVisitor* semantics,
       TranslationUnitRequest* translationUnit,
       SourceLanguage sourceLanguage,
       TokenSpan const& tokens,
       DiagnosticSink* sink,
       Scope* currentScope,
       Scope* outerScope);
   ```

The two entry points differ mainly in the `ParsingStage` they record in
the parser's `ParserOptions`: `parseSourceFile` selects
`ParsingStage::Decl`, `parseUnparsedStmt` selects `ParsingStage::Body`.
Code that must behave differently between the two stages consults
`Parser::getStage()`.

This two-stage scheme exists primarily to disambiguate `<` between a
generic argument list and a less-than comparison: at body-parse time
`tryParseGenericApp`
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
2951-2995) can hand the expression before the `<` to the checker's
`CheckTerm` and act on what it names. The test is wider than "is this
a generic": a base that resolves to a `GenericDecl`, to a
`FunctionDeclBase`, _or_ to an `AggTypeDeclBase` all commit to the
generic reading, because a function or type name can never legally
precede a `<` in a comparison and the generic reading yields the
better diagnostic; an overloaded base commits if any candidate is one
of those three. Only a base that resolves to something else — an
ordinary variable, say — forces the comparison reading. The historical
narrative and details are in
[../../../design/parsing.md](../../../design/parsing.md); this document
does not duplicate it.

### Syntax-as-declaration

Slang treats most "keywords" not as lexer-level reserved words but as
identifiers bound to syntax in the active environment. The parser keeps
a `SyntaxParseInfo` table that maps keyword names to parse callbacks;
the type and its accessor are declared in
[slang-parser.h](../../../../source/slang/slang-parser.h) (lines 45-53):

```cpp
struct SyntaxParseInfo
{
    const char* keywordName;
    SyntaxParseCallback callback;
    SyntaxClass<NodeBase> classInfo;
};

ConstArrayView<SyntaxParseInfo> getSyntaxParseInfos();
```

The table itself is the static array `g_parseSyntaxEntries[]` in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp) (line
10700), whose rows are built by the small helpers `_makeParseDecl`,
`_makeParseModifier`, and `_makeParseExpr` (lines 10663-10697);
`getSyntaxParseInfos()` (line 10869) hands the array out as a
`ConstArrayView`.

When the parser sees an identifier it looks it up in the active scope
chain (`tryLookUpSyntaxDecl`, line 1115); if the lookup yields a
`SyntaxDecl` registered through this table, `tryParseUsingSyntaxDecl`
(line 1202) invokes the associated callback. That is the first thing
`ParseDeclWithModifiers` (line 5805) tries for an identifier-initial
declaration, and the same mechanism dispatches modifier keywords from
`ParseModifiers` (line 1229). Most language modifier keywords are
populated this way at startup (`populateBaseLanguageModule`, line 10874) and the core module's `*.meta.slang` files contribute additional
entries. The inventory of keywords is in
[../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md).

The practical consequence: adding a new modifier keyword in Slang is
typically a matter of registering it in the syntax table, not of
touching the lexer or parser core.

### Angle-bracket annotations

Two independent places discard a `< ... >` clause without interpreting
any of it. Both are inherited from legacy D3D effect syntax, and both
throw the clause away entirely — nothing inside the angle brackets
reaches the AST.

The _declarator-level_ skip, in `parseDirectAbstractDeclarator`
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
2640-2681), is opt-in: it runs only when
`ParserOptions::enableEffectAnnotations` is set, which
`-enable-effect-annotations` does (line 9982). Because a `<` after a
declarator is otherwise a generic argument list, it disambiguates
first — `<let` and a `< X :` prefix are still generic argument lists —
and otherwise scans ahead on a scratch `TokenReader` for a `;` before
the next `>`. Finding one identifies the clause as an annotation, so
the parser commits the scratch reader and reads the `>`; finding none
leaves the tokens for the generic-argument path.

The _semantic-level_ skip, in `_parseOptSemantics` (lines 3966-3978),
is unconditional. After a semantic such as `: SV_Position`, a
following `<` is always treated as an annotation: every token up to
the next `>` is advanced past, with no flag and no disambiguation.

### Error recovery

When the parser hits an unexpected token, `Unexpected`
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
322-367) emits a diagnostic through the `DiagnosticSink` (see
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)) and
sets `Parser::isRecovering`. That flag is what suppresses a cascade:
while it is set no further "unexpected token" diagnostic is reported,
and it is cleared as soon as a token the parser was waiting for turns
up.

Resynchronization is done by `TryRecover` (line 483), which takes a
_recover-before_ set (tokens to stop in front of, leaving them
unconsumed) and a _recover-after_ set (tokens to consume and then
continue past). The default strategy used inside `{ ... }` blocks
(line 628) is recover-before `}` and recover-after `;`. Skipping is
done a _balanced group_ at a time by `SkipBalancedToken` (line 381), so
a bracketed or braced region is stepped over whole rather than
token-by-token, and `TryRecover` refuses to skip past a closing token
(`)`, `]`, `}`, end-of-file — see `IsClosingToken`, line 420) unless a
closing token is itself what it is looking for. The AST is built
best-effort even after errors so that downstream tools can still operate
on a partial tree.

That block strategy is the _only_ place a recover-after set is used.
Every other recovery site goes through `TryRecoverBefore` (line 621),
which passes a single recover-before token and no recover-after set at
all, so outside a block the parser resynchronizes on a closing token
rather than on a separator. There are two such sites:

- `Parser::readTokenImpl` (line 635) recovers before the token it was
  expecting — but only once `isRecovering` is already set, or when the
  expected token is a `}` / `)` / `]` read through `ReadMatchingToken`.
  A first unexpected token in a `ReadToken` call is reported and left
  in place.
- `AdvanceIfMatch` (line 805), which drives every `( ... )`,
  `[ ... ]`, `{ ... }` and file-scope list, recovers before that
  region's closing token. If the next token is instead in the region's
  _bail_ set — `}` or end-of-file for `( ... )` and `[ ... ]`,
  end-of-file alone for `{ ... }` and file scope
  (`kMatchedTokenInfos`, line 783) — it abandons the search and lets
  the enclosing construct close instead.

So a parameter list, an initializer list, and a declaration position
do not have recovery sets of their own: they inherit the closing
token of whichever matched region encloses them.

## AST data model

The AST is a strongly-typed C++ class hierarchy rooted at `NodeBase`,
declared in
[slang-ast-base.h](../../../../source/slang/slang-ast-base.h):

```cpp
FIDDLE(abstract)
class NodeBase
{
    FIDDLE(...)
    // ...
    ASTNodeType astNodeType = ASTNodeType(-1);
    ASTBuilder* getASTBuilder();
};
```

The `FIDDLE(...)` macro instances are processed by the build-time
`slang-fiddle` tool, which generates the matching definitions under
`build/source/slang/fiddle/` (e.g.
`slang-ast-base.h.fiddle`). The generated code provides the visitor
dispatch table, the `SyntaxClass` reflection metadata used by `as<T>`
casts, and serialization support. **Do not edit the generated files;
edit the FIDDLE-marked source.**

### The major node families

The _base_ class of every family is declared alongside `NodeBase` in
[slang-ast-base.h](../../../../source/slang/slang-ast-base.h); each
family's _concrete_ subclasses then live in their own `slang-ast-*.h`
header ([../architecture/module-map.md](../architecture/module-map.md)
lists them all):

- `Decl` — declarations (base at
  [slang-ast-base.h](../../../../source/slang/slang-ast-base.h) line
  763). Concrete and intermediate declaration classes are in
  [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h):
  `ContainerDecl`, `FunctionDeclBase`, `VarDecl`, `AggTypeDecl`,
  `SimpleTypeDecl`, `GenericDecl`, `ExtensionDecl`, `InterfaceDecl`,
  `SyntaxDecl`, ...
- `Expr` — expressions (base at line 815); subclasses in
  [slang-ast-expr.h](../../../../source/slang/slang-ast-expr.h).
- `Stmt` — statements (base at line 825); subclasses in
  [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h),
  including `UnparsedStmt` for the deferred-body case described above.
- `Type` — types (base at line 569; note that `Type` derives from
  `Val`, so a type _is_ a compile-time value); subclasses in
  [slang-ast-type.h](../../../../source/slang/slang-ast-type.h).
- `Modifier` — qualifiers / attributes attached to decls (base at line
  707); subclasses in
  [slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h).
- `Val` — compile-time values used by generics (base at line 380), with
  concrete value subclasses in
  [slang-ast-val.h](../../../../source/slang/slang-ast-val.h). Its
  user-level surface is a generic _value_ argument: the `3` in
  `vector<float, 3>` is the `IntVal` that
  `VectorExpressionType::getElementCount` returns, and the `4` in
  `int a[4]` is the one `ArrayExpressionType::getElementCount` returns
  ([slang-ast-type.h](../../../../source/slang/slang-ast-type.h) lines
  583 and 751).

In a freshly-parsed AST, both types and expressions use `Expr`
representation (since at parse time `A(B)` could resolve to a function
call or a type construction); the semantic checker
([03-semantic-check.md](03-semantic-check.md)) is what re-classifies
them. The checker may also rewrite a parsed node into a more
specialized one: for example, an `InvokeExpr` whose callee is a builtin
arithmetic / comparison / bitwise / shift / unary operator over builtin
scalar, vector, or matrix operands is converted to a
`BuiltinOperatorExpr`
([slang-ast-expr.h](../../../../source/slang/slang-ast-expr.h)) by
`convertToBuiltinArithmeticOp`
([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)),
which records the resolved `BuiltinOperationKind` so the operator-name →
kind mapping happens exactly once instead of being re-parsed by each
consumer (constant folding, IR lowering, for-loop trip-count
inference). This node is synthesized during checking, not produced by
the parser.

AST node casts use the templated `as<T>` helpers in
[slang-ast-base.h](../../../../source/slang/slang-ast-base.h):

```cpp
template<typename T>
T* as(NodeBase* node);
```

These dispatch on the FIDDLE-generated `SyntaxClass` metadata rather
than C++ RTTI.

### `ASTBuilder`

[slang-ast-builder.h](../../../../source/slang/slang-ast-builder.h) /
[slang-ast-builder.cpp](../../../../source/slang/slang-ast-builder.cpp).

`ASTBuilder` owns:

- Allocation. AST nodes are arena-allocated through the builder so
  that lifetime tracks the owning module / session.
- Hash-consing of types. Two structurally-identical types are
  represented by the same `Type*` pointer; the builder maintains the
  hash-cons table.

The split is visible in the API. `ASTBuilder::create<T>()` allocates a
fresh node and is what the parser calls for declarations, expressions,
and statements; a `Val` type fails to compile there, rejected by a
`static_assert`. Anything in the `Val` /
`DeclRef` world instead goes through `getOrCreate<T>()`, which keys the
node on its operands in the `m_cachedNodes` dictionary and returns the
existing node on a hit. Some of the typed wrappers around
`getOrCreate` also canonicalize before interning, so that one logical
value has one representation: `getTypeCastIntVal` unwraps a nested
`TypeCastIntVal`, tries to constant-fold the cast, and drops the cast
entirely when the operand already has the target type, rather than
always producing a fresh `TypeCastIntVal` wrapper.

Interning is visible from Slang source as type identity. The core
module declares `typedef vector<float,3> float3;`
([core.meta.slang](../../../../source/slang/core.meta.slang) line
2594), and because both spellings intern to the same `Type*`,
`int probe(float3 v)` and `int probe(vector<float, 3> v)` are one
signature: the second is a redeclaration of the first, not a second
overload.

Because `ASTBuilder*` is needed to construct any AST node, the builder
pointer is threaded through every parsing helper that produces a node.

## Generics ambiguity

A bare `<` after an identifier is syntactically ambiguous: it can
start a generic argument list (`foo<T>`) or be the less-than
operator (`foo < bar`). `tryParseGenericApp` resolves it by
speculating on a _copy_ of the `Parser` (with a throwaway
`DiagnosticSink`), so the real token reader never moves; only if the
speculative parse is error-free and the token that follows the closing
`>` is in the generic-application FOLLOW set — `::`, `.`, `(`, `)`,
`[`, `]`, `:`, `,`, `?`, `;`, `==`, `!=`, `>`, `>>`, or end-of-file
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) lines
3033-3052), the same list given in
[grammar.md's `<` disambiguation section](../syntax-reference/grammar.md#-disambiguation)
— does it reparse the generic application on the real parser.
Otherwise the `<` is still unread and ordinary infix parsing takes
over — there is no single-token lookahead heuristic that suffices in
all cases. Generic _declarations_ are unambiguous because declaration
context tells the parser that `<` opens a parameter list: both a
keyword-led declaration (`func`, `struct`, `interface`, …) and a
C-style function declarator such as `float f<T>(T x)`, where the `<`
after the declarator is itself what selects the function branch, end
up introducing a `GenericDecl`. The parser collects the parameters
into a `GenericDecl` and continues with the inner declaration.

An optional `where` clause may follow the parameter list to attach
type constraints. Parsing only records the syntactic form; constraint
solving and substitution happen during checking
([slang-check-constraint.cpp](../../../../source/slang/slang-check-constraint.cpp))
and IR specialization (see
[05-ir-passes.md](05-ir-passes.md)).

Constraints are parsed by `parseOptionalGenericConstraints` (the
inheritance-clause form, `: Base1, Base2`) and
`maybeParseGenericConstraints` (the `where` form), both in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp). Each
produces `GenericTypeConstraintDecl` members
([slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h)). For a
generic, those constraint decls are siblings of the parameters under
the enclosing `GenericDecl`. `parseOptionalGenericConstraints` takes an
optional `constraintTarget` parameter that decouples the constrained
subject (derived from the decl being parsed) from the container the
constraint decls are added to.

`maybeParseGenericConstraints` accepts several spellings after `where`,
each mapping to its own constraint decl class
([slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h)):

- a type/witness bound (`T : IBar`, `T == U`) → `GenericTypeConstraintDecl`
  (the `==` spelling sets `isEqualityConstraint`; the `:` spelling
  accepts a comma-separated list of bounds and produces one decl per
  bound);
- `nonempty(Pack)` → `NonEmptyPackConstraintDecl`;
- `countof(Pack) == IntExpr` → `GenericVariadicPackCountConstraintDecl`,
  parsed by `_parsePackCountConstraintCountOfExpr` plus
  `_readPackCountConstraintOperator`. This spelling is one-sided on
  purpose: the reversed form `IntExpr == countof(Pack)` is detected by
  `_hasCountOfOnRightOfPackCountComparison` and rejected with
  `Diagnostics::VariadicPackCountConstraintRequiresCountofOnLeft` rather
  than misread as an unrelated type constraint;
- `__hasDiffTypeInfo(T)` → `HasDiffTypeInfoConstraintDecl`;
- a coercibility bound `To(From)` → `TypeCoercionConstraintDecl`, with a
  trailing `implicit` keyword attaching an `ImplicitConversionModifier`.

The type/witness, `nonempty`, and `countof` forms may be prefixed with
`optional`, which attaches an `OptionalConstraintModifier`. `optional`
on a `__hasDiffTypeInfo` clause is rejected with
`Diagnostics::OptionalHasDiffTypeInfoConstraintIsInvalid`.

`FuncConstraintDecl`
([slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h)) is a
subclass of `GenericTypeConstraintDecl` that the parser never produces:
header checking synthesizes it as a sibling requirement when an
interface's callable requirement must itself be constrained as a type
(for example a `[Differentiable]` method). It carries an extra checked
`callableRequirementDeclRef` naming the callable being constrained.

### Interface associated-type and `__constraint` requirements

When an `associatedtype` is declared inside an interface, its
constraints — whether written as an inheritance clause
(`associatedtype A : IBar`) or a `where` clause
(`associatedtype A where A : IBar`) — are relocated to the enclosing
`InterfaceDecl` so that they become interface-level requirements,
siblings of the associated type. `parseAssocType`
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 4293) detects the enclosing interface through `parser->currentScope` and
passes it as the `constraintTarget`. This yields the same representation
as the `__constraint` syntax-as-declaration form:

```slang
interface IDerived : IBase { __constraint DataType == This; }
```

The `__constraint` keyword is registered in the syntax-parse table
(`g_parseSyntaxEntries` in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp), line 10705) and handled by `parseInterfaceConstraintDecl` (line 4335), which
builds a `GenericTypeConstraintDecl`, parsing the subject type, then
either `==` (setting `isEqualityConstraint`) or `:` (a subtype
requirement), then the bound type. Where a `__constraint` is legal is
enforced centrally by `isDeclAllowed` (line 5346), which permits a
`GenericTypeConstraintDecl` only as a member of an `InterfaceDecl` or a
`GenericDecl`.

The deeper treatment of the disambiguation strategy lives in
[../../../design/parsing.md](../../../design/parsing.md).

## Modifier parsing

Modifiers (`in`, `out`, `static`, `const`, ...) and attributes
(`[unroll]`, `[shader("compute")]`, ...) attach to a `Decl` through
the `Modifier` chain rooted at `ModifiableSyntaxNode`. `ParseModifiers`
([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 1237) collects modifier tokens before the declaration keyword and
attaches them as `Modifier` nodes — keyword modifiers through the same
syntax-decl lookup described above, and `[...]` groups through
`ParseSquareBracketAttributes` (line 996). Semantic checking later
validates them against the kind of declaration they modify
(see [03-semantic-check.md](03-semantic-check.md)). That split is
where the parse-stage / check-stage boundary sits: `[unroll]` written
in front of a function rather than a loop parses without complaint —
there is no unexpected-token report — and the only diagnostic is the
checker's `Diagnostics::AttributeNotApplicable` (E31002, "attribute
'unroll' is not valid here").

One placement rule is enforced by the parser itself rather than
deferred. `Parser::ParseStruct` (line 6370) still parses a bracketed
attribute list written _after_ the `struct` keyword
(`struct [attr] Name`), but gates it on the module's language version:
silently accepted before 2025, reported as
`Diagnostics::DeprecatedBracketAttributesPlacement` (W31204) at 2025,
and as `Diagnostics::InvalidBracketAttributesPlacement` (E31205) from
2026 on. Attributes written before the keyword are unaffected.

The list of modifier classes is in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h);
attribute parsing is just modifier parsing in disguise — the
`[name(args)]` tokens become an `AttributeBase` modifier. Adding an
attribute that needs its own semantics is therefore a matter of giving
it a dedicated `Attribute` subclass; for instance `NoDiscardAttribute`
(the `[NoDiscard]` attribute, which causes the checker to error when a
call to the marked function appears in a result-discarding context such
as an expression statement).

## Failure modes

- Token-level errors not previously reported by the lexer (e.g. an
  unrecognized punctuator inside a declaration) surface here as parse
  errors via the `DiagnosticSink`.
- Heuristic disambiguation can be wrong; in those cases the parser
  prefers to produce _some_ AST and let the checker either succeed or
  emit a more specific error, rather than aborting parsing.
- Some constructs parse cleanly but are still diagnosed because they
  could never be referenced. `maybeDiagnoseKeywordUsedAsName`
  ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp) line 2529) warns (`Diagnostics::KeywordUsedAsName`) when a declarator name
  is a reserved type keyword (`struct`, `class`, `enum`, `typealias`,
  `typedef`; see `isReservedKeywordName`, line 2510). Both this check
  and the operator-name rule below run from the shared declarator path,
  so they cover `var`, `let`, parameter, field, and `typedef`
  declarations uniformly.
- An `operator <op>` name is only legal for a function. The rule is
  enforced in `UnwrapDeclarator` (line 2752), the single point every
  C-style declarator passes through on its way to a declaration: the
  `isOperatorName` flag that `parseDirectAbstractDeclarator` recorded is
  reported as `Diagnostics::OperatorNameOnNonFunction` unless the caller
  opted in with `allowOperatorName`, which only the function branch of
  `ParseDeclaratorDecl` (line 3584) does, at line 3697 — the branch
  reached once a parameter list or generic `<` has confirmed the
  declarator is a function. So `V operator+(V a, V b) { ... }` is
  accepted, while `int operator+ = 3;` — the same name on a variable
  declarator — draws `Diagnostics::OperatorNameOnNonFunction` (E20020).
  Because the check sits at that chokepoint, the variable, parameter,
  `typedef`, and property cases are rejected without a per-kind test.
  A malformed `operator <garbage>` is not flagged twice — it has
  already produced `Diagnostics::InvalidOperator`.
- The statement parser accepts only a subset of declaration forms.
  `Parser::parseVarDeclrStatement` (line 7255) parses a declaration
  through the ordinary declaration path and then keeps it only if it is
  a variable (`VarDeclBase`), a `DeclGroup`, an aggregate type
  (`AggTypeDecl` — `struct`, `class`, `enum`, `interface`), a
  `typedef` / `typealias` (`TypeDefDecl`), or a `using`. Anything else
  written inside a function body — a `namespace`, for example — is
  reported with `Diagnostics::DeclNotAllowed` (E30102,
  "namespace is not allowed here."). This is a separate mechanism from
  the container-nesting check `isDeclAllowed` performs for declarations
  written inside another declaration.
- Literal _expressions_ are where token text finally becomes a value, so
  some literal diagnostics are parse-time rather than lex-time.
  `parseFloatingPointLiteralExpr` (line 8715) asks
  `getFloatingPointLiteralValue` for a value plus a
  `FloatingPointLiteralType` classification
  ([slang-lexer.h](../../../../source/compiler-core/slang-lexer.h)), and
  maps that classification to the `FloatingPointLiteralExpr`'s
  `suffixType` (`Half` / `Float` / `Double`) or to a diagnostic
  (`InvalidFloatingPointLiteralNumber` for a bad significand,
  `InvalidFloatingPointLiteralSuffix` for an unrecognized suffix). Range
  and precision problems are reported by the same helper's
  `outIsOutOfRange` / `outPrecisionLost` flags, which the parser turns
  into `FloatLiteralTooSmall`, `FloatLiteralUnrepresentable`, or
  `FloatHexLiteralPrecisionLost`. The parser reports at most one of
  these per literal: a single `diagnosed` flag guards the later arms,
  so the classification reports come first, then the range pair, then
  the precision report. The two classification arms are mutually
  exclusive (one `switch` over `FloatingPointLiteralType`), so the only
  way to trip two conditions is a classification failure that is also
  flagged out of range — `1e400q`, an unrecognized suffix on a
  significand that overflows — and there the suffix report wins and the
  range report is suppressed. The parser stores the value the helper
  returned; it does no clamping or truncation of its own. The
  lexer-side half of this split (scanning versus decoding) is described
  in [01-lex-preprocess.md](01-lex-preprocess.md).
- Integer literals are decoded on the same schedule.
  `parseIntegerLiteralExpr` (line 8608) splits the suffix into a width
  part (`l` / `L`, `ll` / `LL`, `z` / `Z`) and an unsigned part
  (`u` / `U`) — repeating either draws
  `Diagnostics::InvalidIntegerLiteralSuffix` — and
  `_determineIntegerLiteralType` (line 8486) maps the pair plus the
  magnitude onto a base type. The pointer-width suffix `z` selects
  `intptr_t`, or `uintptr_t` when `u` is also present; for a
  non-decimal literal it does so without consulting the magnitude at
  all, so `0xFFz` is `intptr_t`. For a decimal `z` (or `ll`) literal
  with no `u`, a value up to `INT64_MAX` is signed; exactly
  `INT64_MAX + 1` is typed unsigned but marked
  `signedMinimumIntException`, which lets a surrounding unary `-` in
  `parsePrefixExpr` (line 9835) rewrite the type back to `intptr_t` /
  `int64_t`; and `INT64_MAX + 2` and above warns
  `Diagnostics::IntegerLiteralTooLarge` (W40004) and stays unsigned.
- The grammar that the parser actually accepts is reverse-engineered
  in [../syntax-reference/grammar.md](../syntax-reference/grammar.md).

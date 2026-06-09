---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-05T09:24:37Z
source_commit: 52339028a2aa703271533454c6b9528a534bac31
watched_paths_digest: d21a76a8273d89c3084ce7ab14317bb6c3306843578006bdf09a9bc0860cfb4a
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
`ParsingCursor` save / restore for backtracking — used by the
heuristic disambiguator described below.

### Two-stage parsing

Slang parses in two stages:

1. **Decl-parse stage.** Top-level declarations are parsed normally,
   but when the parser reaches a `{ ... }` function body it does not
   recurse into it; instead, it captures the enclosed token range as
   an `UnparsedStmt` AST node.
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

This two-stage scheme exists primarily to disambiguate `<` between a
generic argument list and a less-than comparison: at body-parse time
the checker can tell the parser whether the token before `<`
type-checks as a generic-typed declaration. The historical narrative
and details are in [../../../design/parsing.md](../../../design/parsing.md);
this document does not duplicate it.

### Syntax-as-declaration

Slang treats most "keywords" not as lexer-level reserved words but as
identifiers bound to syntax in the active environment. The parser
keeps a `SyntaxParseInfo` table
([slang-parser.h](../../../../source/slang/slang-parser.h),
[slang-syntax.h](../../../../source/slang/slang-syntax.h),
[slang-syntax.cpp](../../../../source/slang/slang-syntax.cpp)) that maps
keyword names to parse callbacks:

```cpp
struct SyntaxParseInfo
{
    const char* keywordName;
    SyntaxParseCallback callback;
    SyntaxClass<NodeBase> classInfo;
};

ConstArrayView<SyntaxParseInfo> getSyntaxParseInfos();
```

When the parser sees an identifier it looks it up in the active scope
chain; if the lookup yields a `SyntaxDecl` registered through this
table, the parser invokes the associated callback. Most language
modifier keywords are populated this way at startup
(`populateBaseLanguageModule` in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp)) and the
core module's `*.meta.slang` files contribute additional entries. The
inventory of keywords is in
[../syntax-reference/keywords-and-builtins.md](../syntax-reference/keywords-and-builtins.md).

The practical consequence: adding a new modifier keyword in Slang is
typically a matter of registering it in the syntax table, not of
touching the lexer or parser core.

### Error recovery

When the parser hits an unexpected token, it emits a diagnostic
through the `DiagnosticSink` (see
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md))
and uses simple skip-to-synchronization-point heuristics (looking for
`;`, `}`, or the next declaration keyword) to resume. The AST is
built best-effort even after errors so that downstream tools can still
operate on a partial tree.

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

Each family has its own `slang-ast-*.h` header
([../architecture/module-map.md](../architecture/module-map.md) lists
them all):

- `Decl` — declarations; root of the
  [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h)
  hierarchy. Includes `ContainerDecl`, `FunctionDeclBase`, `VarDecl`,
  `TypeDecl`, `GenericDecl`, `ExtensionDecl`, `InterfaceDecl`,
  `SyntaxDecl`, ...
- `Expr` — expressions; rooted in
  [slang-ast-expr.h](../../../../source/slang/slang-ast-expr.h).
- `Stmt` — statements; rooted in
  [slang-ast-stmt.h](../../../../source/slang/slang-ast-stmt.h),
  including `UnparsedStmt` for the deferred-body case described above.
- `Type` — types; rooted in
  [slang-ast-type.h](../../../../source/slang/slang-ast-type.h).
- `Modifier` — qualifiers / attributes attached to decls; rooted in
  [slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h).
- `Val` — compile-time values used by generics; rooted in
  [slang-ast-val.h](../../../../source/slang/slang-ast-val.h).

In a freshly-parsed AST, both types and expressions use `Expr`
representation (since at parse time `A(B)` could resolve to a function
call or a type construction); the semantic checker
([03-semantic-check.md](03-semantic-check.md)) is what re-classifies
them. AST node casts use the templated `as<T>` helpers in
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

Because `ASTBuilder*` is needed to construct any AST node, the builder
pointer is threaded through every parsing helper that produces a node.

## Generics ambiguity

A bare `<` after an identifier is syntactically ambiguous: it can
start a generic argument list (`foo<T>`) or be the less-than
operator (`foo < bar`). The parser resolves the ambiguity by
attempting a generic-application parse and rolling back to the
expression parse if it does not commit cleanly — there is no
single-token lookahead heuristic that suffices in all cases. Generic
*declarations* are unambiguous because they appear after a
declaration keyword (`func`, `struct`, `interface`, …) where `<`
can only begin a parameter list; the parser collects the parameters
into a `GenericDecl` and continues with the inner declaration.

An optional `where` clause may follow the parameter list to attach
type constraints. Parsing only records the syntactic form; constraint
solving and substitution happen during checking
([slang-check-constraint.cpp](../../../../source/slang/slang-check-constraint.cpp))
and IR specialization (see
[../pipeline/05-ir-passes.md](05-ir-passes.md)).

The deeper treatment of the disambiguation strategy lives in
[../../../design/parsing.md](../../../design/parsing.md).

## Modifier parsing

Modifiers (`in`, `out`, `static`, `const`, ...) and attributes
(`[unroll]`, `[shader("compute")]`, ...) attach to a `Decl` through
the `Modifier` chain rooted at `ModifiableSyntaxNode`. The parser
collects modifier tokens before the declaration keyword and attaches
them as `Modifier` nodes; semantic checking later validates them
against the kind of declaration they modify
(see [03-semantic-check.md](03-semantic-check.md)).

The list of modifier classes is in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h);
attribute parsing is just modifier parsing in disguise — the
`[name(args)]` tokens become an `AttributeBase` modifier.

## Failure modes

- Token-level errors not previously reported by the lexer (e.g. an
  unrecognized punctuator inside a declaration) surface here as parse
  errors via the `DiagnosticSink`.
- Heuristic disambiguation can be wrong; in those cases the parser
  prefers to produce *some* AST and let the checker either succeed or
  emit a more specific error, rather than aborting parsing.
- The grammar that the parser actually accepts is reverse-engineered
  in [../syntax-reference/grammar.md](../syntax-reference/grammar.md).

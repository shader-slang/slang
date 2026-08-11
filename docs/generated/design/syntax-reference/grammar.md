---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:24:33Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: da0d1f10369218c4e091cd72414d8d541941c541ee85497b27e089007588104f
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Slang Grammar (Reverse-Engineered)

This document is an EBNF-style approximation of the surface syntax
that Slang's parser accepts. It is **reverse-engineered** from
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp), not
designed; mismatches with the implementation are bugs in this
grammar. The intended reader is a tooling developer (syntax
highlighter, formatter, language server) who needs an approximate
grammar for offline reasoning.

## Caveats

- Slang has no formal grammar. Several productions below are
  context-sensitive and the parser disambiguates them with
  heuristics, lookups in the active environment, or the two-stage
  parsing strategy described in
  [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md). Such
  productions are flagged inline with a "context-sensitive" note.
- The token-level vocabulary (terminals) is the catalog in
  [tokens.md](tokens.md). The keywords used as terminals come from
  [keywords-and-builtins.md](keywords-and-builtins.md).
- Functions, methods, and other declaration bodies are captured as
  raw token spans in stage 1 and re-parsed during checking. This
  document describes the surface syntax of bodies as if they were
  parsed in one pass.

## Notation

```
RULE        ::= ALTERNATIVES
ALTERNATIVES ::= ALT ('|' ALT)*
A?          – zero or one A
A*          – zero or more A
A+          – one or more A
(A B)       – grouping
'foo'       – literal token text (terminal)
KIND        – a token kind from tokens.md (terminal)
```

Identifiers in `UpperCamelCase` are non-terminals defined in this
document. Terminals are either literal strings (the spelling of the
keyword or operator) or short aliases for token kinds from
[tokens.md](tokens.md):

| Alias used here | `TokenType` in tokens.md |
| --- | --- |
| `IDENT` | `Identifier` |
| `INT_LIT` | `IntegerLiteral` |
| `FLOAT_LIT` | `FloatingPointLiteral` |
| `STRING_LIT` | `StringLiteral` |
| `CHAR_LIT` | `CharLiteral` |

The aliases are used purely to keep the grammar tables readable; the
canonical names are the `TokenType` enumerators.

For brevity, every non-terminal cites the parser function that
implements it (in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp)).

## Top-level structure

```
SourceFile      ::= TopDecl* EOF                              -- parseSourceFile (-> parseDecls)
ModuleHeader    ::= 'module' (IDENT | STRING_LIT)? ';'       -- parseModuleDeclarationDecl (one name
                                              --   token, never dotted; omitted = current module)
                  | 'implementing' ModuleName ';'            -- parseImplementingDecl
                                              --   (via parseFileReferenceDeclBase)
ModuleName      ::= IDENT ('.' IDENT)* | STRING_LIT          -- dotted identifier or string literal

TopDecl         ::= ImportDecl                                -- parseDecls -> ParseDecl
                  | ModuleHeader                              -- placement is semantic
                  | NamespaceDecl
                  | UsingDecl
                  | FileDecl
                  | Decl

ImportDecl      ::= ('import' | '__import') ImportPath ';'  -- parseImportDecl
                  | '__include' ImportPath ';'              -- parseIncludeDecl
ImportPath      ::= IDENT ('.' IDENT)* | STRING_LIT          -- parseFileReferenceDeclBase;
                                                              --   dotted identifier or string literal
                                                              --   (the string form is read verbatim by
                                                              --   getFileNameTokenValue; see note)

NamespaceDecl   ::= 'namespace' IDENT (('.' | '::') IDENT)* '{' TopDecl* '}'
                                                              -- parseNamespaceDecl (loops over
                                                              --   '.' / '::' to build a nested chain)
UsingDecl       ::= 'using' 'namespace'? Expr ';'            -- parseUsingDecl (the entity is an
                                                              --   arbitrary expression; valid checked
                                                              --   code is narrower than this parse grammar)
FileDecl        ::= '__file_decl' '{' TopDecl* '}'           -- parseFileDecl

QualifiedName   ::= '::'? IDENT ('::' NameOrSubscript)*       -- parsed via the expression machinery
                                                              --   (parsePostfixExpr -> ParseStaticMemberName
                                                              --   in expression position; parseStaticMemberType
                                                              --   in type position). A leading '::' anchors
                                                              --   lookup at module scope (parseAtomicExpr /
                                                              --   _parseSimpleTypeSpec).
NameOrSubscript ::= IDENT | '__subscript'                     -- ParseStaticMemberName
```

A string-literal module name or import path is *not* unescaped: both
`parseModuleDeclarationDecl` and `parseFileReferenceDeclBase` decode it
with `getFileNameTokenValue`
([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)),
which only strips the surrounding quotes, so the backslash in
`import "foo\bar";` stays literal instead of starting an escape.

`ParseStaticMemberName` accepts the declaration keyword `__subscript`
as a static member name, but only when another `::` follows it, as in
`MyType::__subscript::get`; there the keyword is rewritten to the
internal name a `SubscriptDecl` carries (`getSubscriptOperatorName`), so
the path names the subscript's accessor. Without the trailing `::` the
identifier keeps its ordinary meaning, leaving `MyType::__subscript(i)`
unaffected.

## Declarations

```
Decl            ::= ModifierList? CoreDecl                   -- ParseDecl -> ParseDeclWithModifiers

CoreDecl        ::= TypedefDecl | TypeAliasDecl
                  | StructDecl  | ClassDecl
                  | EnumDecl
                  | InterfaceDecl
                  | ExtensionDecl
                  | GenericDecl
                  | FuncDecl    | ConstructorDecl | SubscriptDecl | PropertyDecl
                  | FuncExtensionDecl
                  | VarDecl     | LetDecl
                  | AssocTypeDecl
                  | RequireCapabilityDecl
                  | SyntaxDecl
                  | AttributeSyntaxDecl
                  | CBufferDecl | TBufferDecl
                  | NamespaceDecl
                  | UsingDecl
                  | InternalDecl
```

`CoreDecl` is dispatched by keyword through the `g_parseSyntaxEntries[]`
table (`ParseDeclWithModifiers` looks the leading keyword up in that
table after `ParseModifiers` has consumed the modifier prefix). Besides
the user-facing forms above, the table registers compiler-internal
declaration keywords that share the same dispatch:

```
InternalDecl    ::= '__constraint' ...                        -- InterfaceConstraintDecl (see above)
                  | '__associatedfunc' ...                     -- parseAssocFunc
                  | 'type_param' ...                           -- parseGlobalGenericTypeParamDecl
                  | '__generic_value_param' ...                -- parseGlobalGenericValueParamDecl
                  | 'semantic' ...                             -- parseSemanticDecl
                  | '__ignored_block' ...                      -- parseIgnoredBlockDecl
                  | '__transparent_block' ...                  -- parseTransparentBlockDecl
```

### Type-defining declarations

```
TypedefDecl     ::= 'typedef' Type Declarator ';'             -- parseTypeDef
                                                              -- the alias name is a full (non-abstract)
                                                              -- declarator, so trailing array suffixes and
                                                              -- the prefix-'*' / parenthesized forms are
                                                              -- folded onto the alias type (see Declarator)
TypeAliasDecl   ::= 'typealias' IDENT ('<' GenericParams '>')? WhereClause? '=' Type ';'
                                                              -- parseTypeAliasDecl

StructDecl      ::= 'struct' Attribute* IDENT? GenericParams? Inheritance?
                    WhereClause? StructBody                   -- Parser::ParseStruct
                  | 'struct' Attribute* IDENT? GenericParams? Inheritance? '=' Type ';'
                                                              -- type-alias form (aliasedType)
                  | 'struct' Attribute* IDENT? ';'            -- forward declaration (hasBody = false)
ClassDecl       ::= 'class'  IDENT Inheritance? StructBody    -- Parser::ParseClass
                                                              -- (no generic params, no 'where'
                                                              --   clause and no forward form)
StructBody      ::= '{' StructMember* '}'                     -- parseDeclBody
StructMember    ::= ModifierList? (VarDecl | FuncDecl | TypedefDecl | TypeAliasDecl
                                  | ConstructorDecl | SubscriptDecl | PropertyDecl
                                  | AssocTypeDecl)

EnumDecl        ::= 'enum' 'class'? IDENT? GenericParams? Inheritance? WhereClause?
                    '{' (EnumCase (',' EnumCase)* ','?)? '}'  -- parseEnumDecl; body may be empty
EnumCase        ::= IDENT ('=' ArgExpr)?                       -- parseEnumCaseDecl; the tag uses
                                                              --   ArgExpr so ',' stays a separator

InterfaceDecl   ::= 'interface' IDENT GenericParams? Inheritance? WhereClause?
                    '{' InterfaceMember* '}'                  -- parseInterfaceDecl
InterfaceMember ::= ModifierList? (FuncDecl | AssocTypeDecl | PropertyDecl
                                  | SubscriptDecl | ConstructorDecl
                                  | InterfaceConstraintDecl)

ExtensionDecl   ::= ('extension' | '__extension') ('<' GenericParams '>')? Type
                    Inheritance? WhereClause? '{' StructMember* '}'  -- parseExtensionDecl

AssocTypeDecl   ::= 'associatedtype' IDENT (':' TypeList)? WhereClause? ';'
                                                              -- parseAssocType
                                                              -- the ':' bound and the 'where'
                                                              -- clause are modeled identically (see below)

InterfaceConstraintDecl
                ::= '__constraint' Type ('==' | ':') Type ';'  -- parseInterfaceConstraintDecl
                                                              -- interface-level constraint requirement;
                                                              -- '==' is an equality constraint,
                                                              -- ':' a subtype constraint

Inheritance     ::= ':' TypeList                              -- parseOptionalInheritanceClause
TypeList        ::= Type (',' Type)*
```

`struct`, `class`, and `enum` are the three declaration keywords that
are *not* registered in `g_parseSyntaxEntries[]`; the type-specifier
parser recognizes them by identifier lookahead and calls
`Parser::ParseStruct`, `Parser::ParseClass`, or `parseEnumDecl`
directly (see [keywords-and-builtins.md](keywords-and-builtins.md)).
That is why the name is optional above — an anonymous one gets a
generated name — and why the declaration can be followed by a
declarator, as in `struct Foo { int x; } foo;`.

The `Attribute*` slot inside `'struct'` is a legacy placement, with the
bracket *after* the keyword. `Parser::ParseStruct` still parses it to
keep diagnostics sensible, but reports
`DeprecatedBracketAttributesPlacement` from language version 2025 and
`InvalidBracketAttributesPlacement` from 2026.

An associated type's constraints have two surface forms — the
inheritance bound `associatedtype A : IBar` and the where-clause
`associatedtype A where A : IBar`. Both are parsed by `parseAssocType`
and lowered to the *same* representation: a `GenericTypeConstraintDecl`
added as a sibling requirement of the enclosing `InterfaceDecl`, not a
member of the associated type itself (see `parseOptionalGenericConstraints`,
which takes a separate `constraintTarget`). This is the identical
representation produced by the explicit `__constraint` form below, so
the three spellings are equivalent.

The standalone `__constraint` requirement (`parseInterfaceConstraintDecl`)
declares a `GenericTypeConstraintDecl` directly in an interface body to
refine the implicit `This` type and/or associated types inherited from
base interfaces — e.g. `interface IDerived : IBase { __constraint DataType == This; }`.
It is valid only inside an interface; `isDeclAllowed` permits a
`GenericTypeConstraintDecl` under an `InterfaceDecl` (interface
requirement) or a `GenericDecl` (where the same node represents a
generic parameter's `where` / `<T : I>` bound).

### Function-style declarations

```
FuncDecl        ::= 'func' IDENT GenericParams? '(' ParamList? ')' ('throws' Type)? ('->' Type)?
                    WhereClause? FuncBody                      -- parseFuncDecl
                  | Type IDENT GenericParams? '(' ParamList? ')' ('throws' Type)?
                    WhereClause? FuncBody                      -- C-style header (context-sensitive)
                                                              -- the leading Type is the return type;
                                                              -- type-vs-name disambiguation is heuristic;
                                                              -- see ../pipeline/02-parse-ast.md
                                                              -- (parseTraditionalFuncDecl)

ConstructorDecl ::= '__init' GenericParams? '(' ParamList? ')'
                    WhereClause? FuncBody                      -- parseConstructorDecl
SubscriptDecl   ::= '__subscript' GenericParams? '(' ParamList? ')' '->' Type
                    WhereClause? AccessorBlock                 -- parseSubscriptDecl
                                                              -- routed through parseOptGenericDecl, so it
                                                              -- accepts inline generic params and a 'where'
                                                              -- clause; an interface subscript may supply
                                                              -- default accessor bodies
PropertyDecl    ::= 'property' IDENT ':' Type AccessorBlock    -- parsePropertyDecl, modern form
                  | 'property' Type Declarator AccessorBlock   -- traditional C form

FuncExtensionDecl
                ::= '__func_extension' GenericParams? KeywordExprHead
                    '(' ParamList? ')' ('throws' Type)? ('->' Type)?
                    WhereClause? FuncBody                      -- parseFuncExtensionDecl
                                                              -- KeywordExprHead is a higher-order
                                                              -- form such as `fwd_diff(foo)`,
                                                              -- `bwd_diff(foo)`, or `__apply(foo)`;
                                                              -- gated behind -experimental-feature

ParamList       ::= Param (',' Param)*                         -- parseParameterList (traditional)
                                                              --   / parseModernParamList (modern)
                  | 'void'                                     -- C-compatibility spelling of an
                                                              --   empty list, accepted only as the
                                                              --   sole content of the parentheses
                                                              --   (parseParameterList)
Param           ::= ModifierList? Type IDENT ('=' ArgExpr)?    -- traditional, type-first
                                                              --   (ParseParameter / _parseTraditionalParamDeclCommonBase)
                  | ModifierList? IDENT (':' Type)? ('=' ArgExpr)? -- modern, name-first
                                                              --   (parseModernParamDecl, chosen when
                                                              --   _peekModernStyleVarDecl succeeds; the
                                                              --   ':' Type is optional)

WhereClause     ::= ('where' WhereTerm)+                       -- maybeParseGenericConstraints
WhereTerm       ::= 'optional'? Type ':' Type (',' Type)*      -- conformance constraint(s)
                  | 'optional'? Type '==' Type                 -- equality constraint
                  | 'optional'? 'nonempty' '(' Expr ')'        -- non-empty pack constraint
                  | 'optional'? 'countof' '(' Expr ')' '==' Expr -- variadic pack-count constraint
                  | '__hasDiffTypeInfo' '(' Type ')'           -- differentiable-type-info constraint
                  | Type '(' Type ')' 'implicit'?              -- type-coercion constraint
                                                              --   (TypeCoercionConstraintDecl: toType '(' fromType ')')
                                                              -- each 'where' introduces one WhereTerm; the
                                                              -- ':' form may list several supertypes. See note.

FuncBody        ::= ';'                                        -- prototype only
                  | '{' BodyTokens '}'                         -- captured as UnparsedStmt in stage 1
BodyTokens      ::= (anything but unbalanced '{' / '}')*       -- see two-stage parsing

AccessorBlock   ::= ';' | '{' AccessorDecl* '}'   -- parseStorageDeclBody; an omitted
                                                  -- block is treated as `{ get; }`
AccessorDecl    ::= ModifierList? AccessorName ('(' ParamList? ')')? FuncBody
                                                  -- parseAccessorDecl; the parameter
                                                  -- list is parsed for any accessor
                                                  -- (it is how `set` names the new
                                                  -- value) and restricted at check time
AccessorName    ::= 'get' | 'set' | 'ref'        -- parseAccessorDecl; any other
                                                  -- accessor name is diagnosed (Unexpected)
```

### Variable / binding declarations

```
VarDecl         ::= 'var' IDENT (':' Type)? ('=' ArgExpr)? ';' -- parseVarDecl
                  | Type VarDeclarator (',' VarDeclarator)* ';' -- C-style; context-sensitive
LetDecl         ::= 'let' IDENT (':' Type)? ('=' ArgExpr)? ';'  -- parseLetDecl; '=' is parser-optional
VarDeclarator   ::= Declarator Initializer?                    -- parseInitDeclarator
ArraySuffix     ::= '[' Expr? ']' ArraySuffix?
Initializer     ::= '=' ArgExpr                                -- parseInitDeclarator (braced = InitListExpr)

Declarator      ::= '*'? (DeclaratorName | '(' Declarator ')') ArraySuffix?
                                                              -- parseDeclarator / UnwrapDeclarator
DeclaratorName  ::= IDENT | 'operator' OperatorName            -- ParseDeclName; the 'operator'
                                                              --   form is legal only for a function
OperatorName    ::= Operator | ',' | '=' | '(' ')' | '?' ':'   -- one operator token, or one of
                                                              --   these three multi-token spellings
```

`Declarator` is the shared C-style name-plus-suffix grammar
(`parseDeclarator`, folded onto a base type by `UnwrapDeclarator`).
It is used by `VarDecl`'s C-style form (through `VarDeclarator`)
and, since the move to the shared machinery, by
`TypedefDecl`: `typedef int arr[2];` and `typedef int* p;` now parse,
with the trailing array suffix and the prefix `*` folded onto the
alias type exactly as for a variable declaration.

An `operator <op>` name is syntactically just another declarator name,
so the parser cannot reject `int operator+ = 10;` while reading the
name. `UnwrapDeclarator` is the single point every C-style declarator
passes through on its way to a declaration — variable, parameter,
`typedef`, `property`, and function all call it — so it is where the
restriction is enforced: it diagnoses `OperatorNameOnNonFunction`
unless the caller passes `allowOperatorName`, which only the function
branch of `ParseDeclaratorDecl` does (the branch taken when the
declarator is followed by a parameter list or a generic `<`).

### HLSL-compatibility declarations

```
CBufferDecl     ::= 'cbuffer' IDENT (':' Register)? '{' StructMember* '}'  -- parseHLSLCBufferDecl
TBufferDecl     ::= 'tbuffer' IDENT (':' Register)? '{' StructMember* '}'  -- parseHLSLTBufferDecl
Register        ::= 'register' '(' RegToken ')'
```

### User-defined syntax

```
SyntaxDecl          ::= 'syntax' IDENT (':' IDENT)? ('=' IDENT)? ';'
                                                              -- parseSyntaxDecl; the ':' clause names
                                                              --   the AST node class to construct and
                                                              --   the '=' clause an existing keyword
                                                              --   to alias (its parse callback is
                                                              --   reused). One is intended, but the parser accepts neither (TODO)
AttributeSyntaxDecl ::= 'attribute_syntax' '[' IDENT AttributeParams? ']' ':' IDENT ';'
                                                              -- parseAttributeSyntaxDecl; the ':'
                                                              --   clause names the AttributeDecl's
                                                              --   syntax class -- there is no '='
                                                              --   alias form for attributes
AttributeParams     ::= '(' AttributeParam (',' AttributeParam)* ')'
AttributeParam      ::= IDENT (':' Type)? ('=' ArgExpr)?      -- parseAttributeParamDecl
RequireCapabilityDecl ::= '__require_capability' CapabilityName (('+' | ',') CapabilityName)* ';'
                                                              -- parseRequireCapabilityDecl; there are
                                                              --   no parentheses, and each name must be
                                                              --   a known capability atom
                                                              --   (findCapabilityName), otherwise
                                                              --   UnknownCapability is reported
CapabilityName        ::= IDENT
```

The statement-level spelling is different: `__requireCapability` inside
a function body *does* take parentheses and only allows commas —
`'__requireCapability' '(' CapabilityName (',' CapabilityName)* ')' ';'`
(`Parser::ParseRequireCapabilityStatement`).

## Statements

Slang's exception-like control flow appears in two distinct places.
`try` is an **expression** keyword (`'try' Expr`, listed under
`KeywordExpr` in the next section); the statement-level handler is
`do ... catch`, modelled after the loop forms. There is no
`try { ... } catch { ... }` statement.

```
Stmt            ::= Block
                  | IfStmt | IfLetStmt
                  | ForStmt | WhileStmt | DoWhileStmt | DoCatchStmt
                  | SwitchStmt | CaseStmt | DefaultStmt
                  | BreakStmt | ContinueStmt | ReturnStmt
                  | DiscardStmt | DeferStmt
                  | ThrowStmt
                  | LabelStmt
                  | CompileTimeForStmt
                  | RequireCapabilityStmt
                  | InternalStmt
                  | DeclStmt | ExprStmt | EmptyStmt

Block           ::= '{' Stmt* '}'                              -- Parser::parseBlockStatement
                                                              -- statement dispatch in ParseStatement
IfStmt          ::= 'if' '(' Expr ')' Stmt ('else' Stmt)?      -- parseIfStatement
IfLetStmt       ::= 'if' '(' 'let' IDENT '=' Expr ')' Stmt ('else' Stmt)?
                                                              -- parseIfLetStatement, selected when
                                                              --   'let' is two tokens after 'if'
ForStmt         ::= 'for' '(' (DeclStmt | ExprStmt | ';') Expr? ';' Expr? ')' Stmt
                                                              -- ParseForStatement
WhileStmt       ::= 'while' '(' Expr ')' Stmt                  -- ParseWhileStatement
DoWhileStmt     ::= 'do' Stmt 'while' '(' Expr ')' ';'         -- ParseDoStatement
DoCatchStmt     ::= 'do' Stmt ('catch' ('(' Param ')')? Stmt)+ -- ParseDoCatchStatement; chained catches nest

SwitchStmt      ::= 'switch' '(' Expr ')' '{' SwitchCase* '}'  -- ParseSwitchStmt
SwitchCase      ::= ('case' Expr ':' | 'default' ':') Stmt*    -- ParseCaseStmt / ParseDefaultStmt

BreakStmt       ::= 'break' IDENT? ';'                          -- ParseBreakStatement
ContinueStmt    ::= 'continue' ';'                             -- ParseContinueStatement
                                                              -- (no optional label, unlike BreakStmt)
ReturnStmt      ::= 'return' Expr? ';'                          -- ParseReturnStatement
DiscardStmt     ::= 'discard' ';'                               -- ParseStatement (inline)
DeferStmt       ::= 'defer' Stmt                                -- ParseDeferStatement
ThrowStmt       ::= 'throw' Expr ';'                            -- ParseThrowStatement
                                                              -- (the ';' is mandatory:
                                                              --   ParseThrowStatement ends with
                                                              --   ReadToken(TokenType::Semicolon))
LabelStmt       ::= IDENT ':' Stmt                              -- parseLabelStatement; chosen when an
                                                              --   identifier is followed by ':'
                                                              --   (the label BreakStmt refers to)
CompileTimeForStmt
                ::= '$' 'for' '(' IDENT 'in' 'Range' '(' Expr (',' Expr)? ')' ')' Stmt
                                                              -- parseCompileTimeStmt reads the '$',
                                                              --   parseCompileTimeForStmt the rest;
                                                              --   'Range' is a required literal keyword
                                                              --   here, and one argument means
                                                              --   Range(end)
RequireCapabilityStmt
                ::= '__requireCapability' '(' CapabilityName (',' CapabilityName)* ')' ';'
                                                              -- ParseRequireCapabilityStatement

DeclStmt        ::= Decl
ExprStmt        ::= Expr ';'                                    -- ParseExpressionStatement
EmptyStmt       ::= ';'
```

The statement dispatcher also recognizes a set of compiler-internal
statement keywords, all matched by direct identifier lookahead in
`Parser::ParseStatement`:

```
InternalStmt    ::= '__target_switch' ...                       -- parseTargetSwitchStmt
                  | '__stage_switch' ...                        -- parseStageSwitchStmt
                  | '__intrinsic_asm' ...                       -- parseIntrinsicAsmStmt
                  | '__GPU_FOREACH' ...                         -- ParseGpuForeachStmt
```

A `try` at statement position is not a statement form of its own:
`ParseStatement` routes it to `ParseExpressionStatement`, because `try`
is an expression keyword (see `KeywordExpr` below).

## Expressions

The expression grammar follows a precedence ladder implemented by a
family of `parse...Expr` functions in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp). Lower
numbers in the table below bind tighter (atom-level), higher numbers
bind looser (assignment).

| Level | Operators | Associativity |
| --- | --- | --- |
| 0 | atoms (literals, names, parenthesized, builtin keyword expressions) | — |
| 1 | postfix `()` `[]` `.` `++` `--` `<...>` (generic specialization, context-sensitive) | left |
| 2 | unary `+` `-` `!` `~` `++` `--` `*` `&` | right |
| 3 | `*` `/` `%` | left |
| 4 | `+` `-` | left |
| 5 | `<<` `>>` | left |
| 6 | `<` `<=` `>` `>=` `is` `as` (right operand is a Type) | left |
| 7 | `==` `!=` | left |
| 8 | `&` | left |
| 9 | `^` | left |
| 10 | `\|` | left |
| 11 | `&&` | left |
| 12 | `\|\|` | left |
| 13 | `?:` ternary | right |
| 14 | `=` `+=` `-=` `*=` `/=` `%=` `<<=` `>>=` `&=` `\|=` `^=` | right |
| 15 | `,` (comma operator; the loosest level, and the default for `ParseExpression`) | left |

The `Precedence` enum ([slang-parser.cpp](../../../../source/slang/slang-parser.cpp))
runs in the opposite direction from the table: `Comma` is its lowest
enumerator and `Postfix` its highest, and `GetOpLevel` maps an operator
token to one of them. `Parser::ParseExpression` takes the level to stop
at, which is how the two comma contexts are distinguished: it defaults
to `Precedence::Comma`, so a full expression *does* admit the comma
operator, while `ParseArgExpr` and `ParseInitExpr` pass
`Precedence::Assignment` so a `,` terminates the operand instead of
being consumed. That is why the `ArgList` and `InitList` productions
below can use `,` as a separator at all.

```
Expr            ::= CommaExpr                                  -- ParseExpression (default level)
CommaExpr       ::= AssignExpr (',' AssignExpr)*               -- parseInfixExprWithPrecedence
ArgExpr         ::= AssignExpr                                 -- ParseArgExpr / ParseInitExpr
                                                              --   (stops at Precedence::Assignment)
AssignExpr      ::= TernaryExpr (AssignOp AssignExpr)?         -- parseInfixExprWithPrecedence
TernaryExpr     ::= LogicalOrExpr ('?' Expr ':' AssignExpr)?   -- parseInfixExprWithPrecedence
LogicalOrExpr   ::= LogicalAndExpr ('||' LogicalAndExpr)*      -- parseInfixExprWithPrecedence
LogicalAndExpr  ::= BitOrExpr ('&&' BitOrExpr)*                -- parseInfixExprWithPrecedence
BitOrExpr       ::= BitXorExpr ('|' BitXorExpr)*               -- parseInfixExprWithPrecedence
BitXorExpr      ::= BitAndExpr ('^' BitAndExpr)*               -- parseInfixExprWithPrecedence
BitAndExpr      ::= EqualityExpr ('&' EqualityExpr)*           -- parseInfixExprWithPrecedence
EqualityExpr    ::= RelationalExpr (('==' | '!=') RelationalExpr)*  -- parseInfixExprWithPrecedence
RelationalExpr  ::= ShiftExpr (('<' | '<=' | '>' | '>=') ShiftExpr | ('is' | 'as') Type)*
                                                              -- parseInfixExprWithPrecedence;
                                                              -- '<' is context-sensitive (generic vs comparison);
                                                              -- 'is' / 'as' are special-cased identifier operators
                                                              --   (IsTypeExpr / AsTypeExpr) whose right operand is
                                                              --   parsed as a Type, not a general expression
ShiftExpr       ::= AddExpr (('<<' | '>>') AddExpr)*           -- parseInfixExprWithPrecedence
AddExpr         ::= MulExpr (('+' | '-') MulExpr)*            -- parseInfixExprWithPrecedence
MulExpr         ::= UnaryExpr (('*' | '/' | '%') UnaryExpr)*  -- parseInfixExprWithPrecedence
UnaryExpr       ::= UnaryOp UnaryExpr | PostfixExpr            -- parsePrefixExpr
UnaryOp         ::= '+' | '-' | '!' | '~' | '++' | '--' | '*' | '&'
PostfixExpr     ::= AtomExpr PostfixSuffix*                    -- parsePostfixExpr
PostfixSuffix   ::= '(' ArgList? ')'                           -- call
                  | '[' ArgList? ']'                            -- subscript; zero or more operands
                  | ('.' | '->') DeclaratorName                 -- MemberExpr / DerefMemberExpr
                  | '::' NameOrSubscript                        -- StaticMemberExpr
                  | '++' | '--'                                 -- postfix inc/dec
                  | GenericSpecialization                       -- '<' Type/Expr (',' Type/Expr)* '>'  context-sensitive
AtomExpr        ::= Literal                                    -- parseAtomicExpr (keyword syntax-decl dispatch)
                  | QualifiedName
                  | '(' Expr ')'                                -- parenthesized (ParenExpr)
                  | '(' Type ')' UnaryExpr                      -- C-style cast (ExplicitCastExpr);
                                                              --   context-sensitive, see note
                  | '(' ')'                                     -- empty tuple; Slang 2026 and later
                  | '(' ArgExpr (',' ArgExpr)+ ')'              -- tuple; Slang 2026 and later
                  | InitListExpr
                  | KeywordExpr
                  | LambdaExpr
                  | NewExpr
                  | SpirvAsmExpr                                -- 'spirv_asm' block (parsePrefixExpr
                                                              --   -> parseSPIRVAsmExpr)
KeywordExpr     ::= 'this'
                  | 'try' UnaryExpr                            -- operand is a leaf (ParseLeafExpression)
                  | 'no_diff' UnaryExpr
                  | ('fwd_diff'|'__fwd_diff') '(' Expr ')'
                  | ('bwd_diff'|'__bwd_diff') '(' Expr ')'
                  | '__apply' '(' Expr ')'                     -- apply-for-backward (experimental)
                  | 'sizeof' '(' ArgExpr (',' ArgExpr)? ')'    -- parseSizeOfExpr; the operand is an
                                                              --   expression (a type name parses as
                                                              --   one) and the optional second
                                                              --   argument selects a data layout
                  | 'alignof' '(' ArgExpr (',' ArgExpr)? ')'   -- parseAlignOfExpr; same shape
                  | 'countof' '(' Expr ')'                     -- parseCountOfExpr
                  | '__dispatch_kernel' '(' ArgExpr ',' ArgExpr ',' ArgExpr ')'  -- function, dispatch size, group size
                  | '__getAddress' '(' Expr ')'
                  | '__floatAsInt' '(' Expr ')'
                  | other __-prefixed compiler-internal forms; see keywords-and-builtins.md
LambdaExpr      ::= '(' ParamList? ')' '=>' (Block | ArgExpr)  -- parseLambdaExpr; there is no
                                                              --   bare-identifier form, and the
                                                              --   parameters use traditional
                                                              --   ParseParameter syntax
NewExpr         ::= 'new' Type ('(' ArgList? ')')?             -- parsePrefixExpr ('new' branch)
Literal         ::= INT_LIT | FLOAT_LIT | STRING_LIT+ | CHAR_LIT
                  | 'true' | 'false'                            -- BoolLiteralExpr
                  | 'nullptr'                                   -- NullPtrLiteralExpr
                  | 'none'                                      -- NoneLiteralExpr
InitListExpr    ::= '{' (ArgExpr (',' ArgExpr)* ','?)? '}'     -- parseAtomicExpr (LBrace case)

AssignOp        ::= '=' | '+=' | '-=' | '*=' | '/=' | '%='
                  | '<<=' | '>>=' | '&=' | '|=' | '^='
```

### Literal forms vs. token kinds

`INT_LIT`, `FLOAT_LIT`, `STRING_LIT`, and `CHAR_LIT` are distinct
`TokenType` values from [tokens.md](tokens.md). The four remaining
literal forms (`true`, `false`, `nullptr`, `none`) have no dedicated
token kind: the lexer emits them as `Identifier`, and the parser
recognises them through entries in the keyword syntax-decl table
(`_makeParseExpr("true", parseTrueExpr)` and friends in
[slang-parser.cpp](../../../../source/slang/slang-parser.cpp)). They
are nonetheless grouped under `Literal` here because they map onto
concrete `LiteralExpr` subclasses in
[slang-ast-expr.h](../../../../source/slang/slang-ast-expr.h)
(`BoolLiteralExpr`, `NullPtrLiteralExpr`, `NoneLiteralExpr`)
alongside `IntegerLiteralExpr`, `FloatingPointLiteralExpr`, and
`StringLiteralExpr`. `CHAR_LIT` is also a `Literal` but lowers to an
`IntegerLiteralExpr` (with `suffixType` `BaseType::UInt`) rather than a
dedicated character-literal node.

`STRING_LIT+` is not a typo: adjacent string-literal tokens are
concatenated into a single `StringLiteralExpr` by the `StringLiteral`
case of `parseAtomicExpr`, so `"a" "b"` is one literal. A numeric
literal's suffix stays part of its token text, so there is no syntactic
distinction between a `half`, `float`, and `double` literal;
`parseFloatingPointLiteralExpr` asks `getFloatingPointLiteralValue`
([slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp))
to classify the suffix and reports a `FloatingPointLiteralType` of
`BadSignificand` or `BadSuffix` as a diagnostic.

### `<` disambiguation

`PostfixExpr` may be followed by `<` to start a generic argument
list. `maybeParseGenericApp` hands the decision to
`tryParseGenericApp`, which uses the strategy described in
[../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md): try to
parse as a generic argument list with a throwaway diagnostic sink, and
if that succeeds, check the token after the matching `>`; if that token
is in the "generic-followers" set (`::`, `.`, `(`, `)`, `[`, `]`, `:`,
`,`, `?`, `;`, `==`, `!=`, `>`, `>>`, end-of-file) treat the `<` as a
generic application, otherwise back out and parse as a comparison.

In body-parse mode (function bodies) a semantic visitor is available,
and it becomes the primary signal: `tryParseGenericApp` checks the base
expression with `CheckTerm` and skips the speculative parse entirely.
A base that resolves to a `GenericDecl` is a generic application; so is
one that resolves to a function or a type even when it is *not* generic,
because a function or type name can never legally precede `<`, and
committing to the generic reading produces a better diagnostic.
Anything else is treated as a comparison.

Inside a generic argument list the ladder itself changes: while
`genericDepth` is non-zero, `GetOpLevel` returns `Precedence::Invalid`
for `>`, `>=`, and `>>`, so those tokens cannot be consumed as
operators and are available to close the argument list. `<`, `<=`, and
the rest keep their normal precedence, which is why a comparison such
as `A<(x > y)>` needs the parentheses.

## Modifiers

```
ModifierList    ::= (Modifier | Attribute)+
Modifier        ::= ModifierKeyword ModifierTail?
ModifierKeyword ::= 'in' | 'out' | 'inout' | 'const' | 'static' | 'inline'
                  | 'public' | 'private' | 'internal' | 'extern' | 'export'
                  | 'uniform' | 'groupshared' | 'precise'
                  | 'nointerpolation' | 'noperspective' | 'linear' | 'sample' | 'centroid'
                  | 'row_major' | 'column_major'
                  | 'point' | 'line' | 'triangle' | 'lineadj' | 'triangleadj'
                  | 'vertices' | 'indices' | 'primitives' | 'payload'
                  | 'override' | 'dynamic_uniform' | 'param' | 'require'
                  | 'dyn' | 'highp' | 'lowp' | 'mediump'
                  | 'volatile' | 'coherent' | 'restrict' | 'readonly' | 'writeonly'
                  | 'shared' | 'layout' | 'hitAttributeEXT'
                  | '__ref' | '__constref' | '__builtin' | '__global' | '__exported'
                  | '__prefix' | '__postfix'
                  | '__intrinsic_op' | '__target_intrinsic'
                  | '__specialized_for_target' | '__attributeTarget'
                  | '__glsl_extension' | '__glsl_version' | '__spirv_version'
                  | '__wgsl_extension' | '__cuda_sm_version'
                  | '__builtin_type' | '__builtin_requirement'
                  | '__magic_type' | '__magic_enum' | '__intrinsic_type'
                  | '__implicit_conversion'

ModifierTail    ::= '(' ArgList? ')'                          -- per-modifier; see keywords-and-builtins.md
```

The complete keyword inventory is in
[keywords-and-builtins.md](keywords-and-builtins.md).

## Attributes and decorations

```
Attribute       ::= '[' AttributeBody (','? AttributeBody)* ']'   -- ParseSquareBracketAttributes
                  | '[[' AttributeBody (','? AttributeBody)* ']]' -- C++-style double bracket
AttributeBody   ::= AttributeName ('(' ArgList? ')')?
AttributeName   ::= '::'? IDENT ('::' IDENT)*                  -- parseAttributeName

ArgList         ::= ArgExpr (',' ArgExpr)*
```

The bracket form `[name(args)]` is identical to a modifier in the
AST representation — `ParseSquareBracketAttributes` constructs
`UncheckedAttribute` nodes that flow through the same `Modifier` chain
as keyword modifiers (see
[../ast-reference/modifiers.md](../ast-reference/modifiers.md)).
The attribute name resolves through the same syntax-decl lookup
the parser uses for keyword modifiers, with `attribute_syntax`
declarations supplying the mapping from name to attribute class.
Inside a single bracket, multiple attributes may appear; the separating
comma is optional, so `[a b]` and `[a, b]` parse the same way.

A `::`-qualified attribute name is not kept as a qualified name.
`parseAttributeName` flattens it into a single synthetic `Identifier`
token by replacing each `::` with `_`, and a *leading* `::` also becomes
a leading `_` — so `[vk::binding(0)]` looks up the name `vk_binding`,
and `[::foo]` looks up `_foo`. Attribute lookup is therefore always a
single-identifier lookup, regardless of how the attribute was spelled.

## Generics and where-clauses

```
GenericDecl     ::= '__generic' '<' GenericParams '>' Decl     -- parseGenericDecl
                                                              --   ALSO inline form on FuncDecl, StructDecl,
                                                              --   InterfaceDecl, ExtensionDecl, TypeAliasDecl
GenericParams   ::= GenericParam (',' GenericParam)*           -- ParseGenericDeclImpl
GenericParam    ::= 'typename'? IDENT (':' Type)? ('=' Type)?  -- type parameter, with an optional
                                                              --   constraint and default argument
                                                              --   (GenericTypeParamDecl)
                  | 'let' IDENT (':' Type)? ('=' Expr)?        -- value parameter
                                                              --   (GenericValueParamDecl)
                  | 'let' 'each' IDENT (':' Type)?             -- value pack parameter
                                                              --   (GenericValuePackParamDecl)
                  | 'each' IDENT (':' Type)?                   -- type pack parameter
                                                              --   (GenericTypePackParamDecl)
                  | 'each'? Type IDENT ('=' Expr)?             -- traditional type-first value
                                                              --   parameter, e.g. `<int N>`
                  | 'functype' IDENT (':' Type)?               -- function-type parameter
                                                              -- all forms: ParseGenericParamDecl

WhereClause     ::= ('where' WhereTerm)+                       -- maybeParseGenericConstraints;
                                                              --   WhereTerm is spelled out under
                                                              --   "Function-style declarations" above
```

The type-parameter and traditional value-parameter forms are
distinguished by two-token lookahead in `ParseGenericParamDecl`: an
identifier followed by `:`, `,`, `>`, or `=` is a type parameter, and
anything else is parsed as a type-first value parameter, so `<T>` and
`<T : IFoo>` declare types while `<int N>` declares a value. Writing
`typename` forces the type reading. The same lookahead (minus `=`)
decides whether `each` introduces a type pack (`each T`) or a
traditional value pack (`each int D`). Note that the `:` constraint on
a generic parameter takes a *single* supertype, unlike the where-clause
`:` form, which accepts a comma-separated list.

Each `where` keyword introduces exactly one `WhereTerm`
(`maybeParseGenericConstraints` loops over `while (AdvanceIf("where"))`);
to state several constraints, repeat the keyword. A leading
`optional` modifier (parsed as `OptionalConstraintModifier`) is
accepted on every term except `__hasDiffTypeInfo`. The
`countof(Pack) == IntExpr` form is *oriented*: the reversed spelling
`N == countof(Pack)` is recognized only to emit a targeted
diagnostic. `nonempty(Pack)` and `countof(Pack) == IntExpr` are
pack-shape constraints on a variadic `each` parameter;
`__hasDiffTypeInfo(Type)` is a compiler-internal differentiability
constraint.

Where-clauses appear after the parameter list (or after the result
clause for function-style declarations) and are syntactically optional
on every kind of generic declaration. The body that follows is
captured as raw tokens during stage-1 parsing and is re-parsed lazily
during checking, so the body sees a fully-resolved generic
parameter list — see
[../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md).

## Types

```
Type            ::= ModifierList? CoreType                     -- ParseType / _parseSimpleTypeSpec
CoreType        ::= QualifiedName (GenericArgs | '.' IDENT)*   -- _parseSimpleTypeSpec; suffixes interleave
                  | CoreType '&' CoreType                       -- AndTypeExpr (_parseInfixTypeExprSuffix)
                  | Type '[' Expr? ']'                          -- array (parsePostfixTypeSuffix)
                  | Type '*'                                    -- pointer-to-T (parsePostfixTypeSuffix)
                  | 'functype' '(' Type (',' Type)* ')' '->' Type
                                                              -- function type (parseFuncTypeExpr)
                  | '__func_as_type' ...                        -- function-as-type reflection
                                                              --   (parseFuncAsTypeExpr)
                  | StructDecl | ClassDecl | EnumDecl           -- an inline type declaration used as
                                                              --   a type specifier, e.g.
                                                              --   `struct Foo { int x; } foo;`
                  | 'each' Type | 'expand' Type                 -- pack types; parsed as prefix
                                                              --   expressions (parseEachExpr /
                                                              --   parseExpandExpr via parsePrefixExpr)
GenericArgs     ::= '<' GenericArg (',' GenericArg)* '>'        -- parseGenericApp; context-sensitive
GenericArg      ::= Type | Expr                                 -- _parseGenericArg; ambiguous,
                                                              --   resolved at check time
```

Built-in concrete type names (`int`, `float`, `vector<T,N>`,
`Texture2D<T>`, `RWStructuredBuffer<T>`, ...) are not part of the
grammar; they are identifiers brought into scope by the meta-modules
documented in [keywords-and-builtins.md](keywords-and-builtins.md).

## Constraints solved at check time

The grammar above intentionally accepts strings the parser will
build into an AST that the semantic checker
([../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md))
later rejects. Examples:

- `Type Identifier` declarations are syntactically valid but the
  decision between "this is a function declaration" and "this is a
  variable declaration" is decided by what follows the identifier;
  the parser uses local lookahead.
- A name that resolves to neither a type nor a function is rejected
  at check time, not at parse time.
- Generic argument lists may contain expressions that look like
  comparisons; the disambiguation note above explains how the
  parser breaks the tie.

These compromises are intentional: deferring the rejection lets the
parser produce a recoverable AST that yields better diagnostics. The
authoritative description of when each construct is rejected is the
checker, not this grammar.

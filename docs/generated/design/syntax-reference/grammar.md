---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-29T13:37:46Z
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
watched_paths_digest: 962d2141d549a42c3603fbba721e8817eaf5daac65cba1a9fd6292074754985b
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
SourceFile      ::= ModuleHeader? TopDecl* EOF                -- parseSourceFile (-> parseDecls)
ModuleHeader    ::= 'module' ModuleName? ';'                 -- parseModuleDeclarationDecl
                                              --   (an omitted name falls back to the
                                              --   current module name)
                  | 'implementing' ModuleName ';'            -- parseImplementingDecl
                                              --   (via parseFileReferenceDeclBase)
ModuleName      ::= IDENT ('.' IDENT)* | STRING_LIT          -- dotted identifier or string literal

TopDecl         ::= ImportDecl                                -- parseDecls -> ParseDecl
                  | NamespaceDecl
                  | UsingDecl
                  | FileDecl
                  | Decl

ImportDecl      ::= ('import' | '__import') ImportPath ';'  -- parseImportDecl
                  | '__include' ImportPath ';'              -- parseIncludeDecl
ImportPath      ::= IDENT ('.' IDENT)* | STRING_LIT          -- parseFileReferenceDeclBase;
                                                              --   dotted identifier or string literal

NamespaceDecl   ::= 'namespace' IDENT (('.' | '::') IDENT)* '{' TopDecl* '}'
                                                              -- parseNamespaceDecl (loops over
                                                              --   '.' / '::' to build a nested chain)
UsingDecl       ::= 'using' 'namespace'? Expr ';'            -- parseUsingDecl (the entity is an
                                                              --   arbitrary expression; valid checked
                                                              --   code is narrower than this parse grammar)
FileDecl        ::= '__file_decl' '{' TopDecl* '}'           -- parseFileDecl

QualifiedName   ::= IDENT ('::' IDENT)*                       -- parsed via the expression machinery
                                                              --   (ParseExpression -> parseStaticMemberType)
```

## Declarations

```
Decl            ::= ModifierList? CoreDecl                   -- parseDecl

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
table (`parseDecl` looks the leading keyword up in that table). Besides
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
TypeAliasDecl   ::= 'typealias' IDENT ('<' GenericParams '>')? '=' Type ';'
                                                              -- parseTypeAliasDecl

StructDecl      ::= 'struct' IDENT GenericParams? Inheritance? StructBody
ClassDecl       ::= 'class'  IDENT GenericParams? Inheritance? StructBody
StructBody      ::= '{' StructMember* '}'
StructMember    ::= ModifierList? (VarDecl | FuncDecl | TypedefDecl | TypeAliasDecl
                                  | ConstructorDecl | SubscriptDecl | PropertyDecl
                                  | AssocTypeDecl)

EnumDecl        ::= 'enum' IDENT (':' Type)? '{' EnumCase (',' EnumCase)* ','? '}'
EnumCase        ::= IDENT ('=' Expr)?

InterfaceDecl   ::= 'interface' IDENT GenericParams? Inheritance?
                    '{' InterfaceMember* '}'                  -- parseInterfaceDecl
InterfaceMember ::= ModifierList? (FuncDecl | AssocTypeDecl | PropertyDecl
                                  | SubscriptDecl | ConstructorDecl
                                  | InterfaceConstraintDecl)

ExtensionDecl   ::= ('extension' | '__extension') Type
                    Inheritance? '{' StructMember* '}'        -- parseExtensionDecl

AssocTypeDecl   ::= 'associatedtype' IDENT (':' TypeList)? WhereClause? ';'
                                                              -- parseAssocType
                                                              -- the ':' bound and the 'where'
                                                              -- clause are modeled identically (see below)

InterfaceConstraintDecl
                ::= '__constraint' Type ('==' | ':') Type ';'  -- parseInterfaceConstraintDecl
                                                              -- interface-level constraint requirement;
                                                              -- '==' is an equality constraint,
                                                              -- ':' a subtype constraint

Inheritance     ::= ':' TypeList
TypeList        ::= Type (',' Type)*
```

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
PropertyDecl    ::= 'property' IDENT ':' Type AccessorBlock    -- parsePropertyDecl

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
Param           ::= ModifierList? Type IDENT ('=' Expr)?       -- traditional, type-first
                                                              --   (ParseParameter / _parseTraditionalParamDeclCommonBase)
                  | ModifierList? IDENT (':' Type)? ('=' Expr)? -- modern, name-first
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

AccessorBlock   ::= ';' | '{' AccessorDecl* '}'
AccessorDecl    ::= ModifierList? AccessorName FuncBody
AccessorName    ::= 'get' | 'set' | 'ref'        -- parseAccessorDecl; any other
                                                  -- accessor name is diagnosed (Unexpected)
```

### Variable / binding declarations

```
VarDecl         ::= 'var' IDENT (':' Type)? ('=' Expr)? ';'    -- parseVarDecl
                  | Type VarDeclarator (',' VarDeclarator)* ';' -- C-style; context-sensitive
LetDecl         ::= 'let' IDENT (':' Type)? '=' Expr ';'        -- parseLetDecl
VarDeclarator   ::= IDENT ArraySuffix? Initializer?
ArraySuffix     ::= '[' Expr? ']' ArraySuffix?
Initializer     ::= '=' Expr | '{' InitList '}'
InitList        ::= Expr (',' Expr)* ','?

Declarator      ::= '*'? (IDENT | '(' Declarator ')') ArraySuffix?  -- parseDeclarator / UnwrapDeclarator
```

`Declarator` is the shared C-style name-plus-suffix grammar
(`parseDeclarator`, folded onto a base type by `UnwrapDeclarator`).
It is used by `VarDecl`'s C-style form (`VarDeclarator` is its
common case) and, since the move to the shared machinery, by
`TypedefDecl`: `typedef int arr[2];` and `typedef int* p;` now parse,
with the trailing array suffix and the prefix `*` folded onto the
alias type exactly as for a variable declaration.

### HLSL-compatibility declarations

```
CBufferDecl     ::= 'cbuffer' IDENT (':' Register)? '{' StructMember* '}'  -- parseHLSLCBufferDecl
TBufferDecl     ::= 'tbuffer' IDENT (':' Register)? '{' StructMember* '}'  -- parseHLSLTBufferDecl
Register        ::= 'register' '(' RegToken ')'
```

### User-defined syntax

```
SyntaxDecl          ::= 'syntax' IDENT '=' QualifiedName ';'    -- parseSyntaxDecl
AttributeSyntaxDecl ::= 'attribute_syntax' '[' IDENT ']' '=' QualifiedName ';'
                                                              -- parseAttributeSyntaxDecl
RequireCapabilityDecl ::= '__require_capability' '(' CapabilityExpr ')' ';'
                                                              -- parseRequireCapabilityDecl
```

## Statements

Slang's exception-like control flow appears in two distinct places.
`try` is an **expression** keyword (`'try' Expr`, listed under
`KeywordExpr` in the next section); the statement-level handler is
`do ... catch`, modelled after the loop forms. There is no
`try { ... } catch { ... }` statement.

```
Stmt            ::= Block
                  | IfStmt | ForStmt | WhileStmt | DoWhileStmt | DoCatchStmt
                  | SwitchStmt | CaseStmt | DefaultStmt
                  | BreakStmt | ContinueStmt | ReturnStmt
                  | DiscardStmt | DeferStmt
                  | ThrowStmt
                  | DeclStmt | ExprStmt | EmptyStmt

Block           ::= '{' Stmt* '}'                              -- parseBlockStmt
                                                              -- statement dispatch in ParseStatement
IfStmt          ::= 'if' '(' Expr ')' Stmt ('else' Stmt)?      -- parseIfStatement
ForStmt         ::= 'for' '(' (DeclStmt | ExprStmt | ';') Expr? ';' Expr? ')' Stmt
                                                              -- ParseForStatement
WhileStmt       ::= 'while' '(' Expr ')' Stmt                  -- ParseWhileStatement
DoWhileStmt     ::= 'do' Stmt 'while' '(' Expr ')' ';'         -- ParseDoStatement
DoCatchStmt     ::= 'do' Stmt 'catch' ('(' Param ')')? Stmt    -- ParseDoStatement / ParseDoCatchStatement

SwitchStmt      ::= 'switch' '(' Expr ')' '{' SwitchCase* '}'  -- ParseSwitchStmt
SwitchCase      ::= ('case' Expr ':' | 'default' ':') Stmt*    -- ParseCaseStmt / ParseDefaultStmt

BreakStmt       ::= 'break' IDENT? ';'                          -- ParseBreakStatement
ContinueStmt    ::= 'continue' ';'                             -- ParseContinueStatement
                                                              -- (no optional label, unlike BreakStmt)
ReturnStmt      ::= 'return' Expr? ';'                          -- ParseReturnStatement
DiscardStmt     ::= 'discard' ';'                               -- ParseStatement (inline)
DeferStmt       ::= 'defer' Stmt                                -- ParseDeferStatement
ThrowStmt       ::= 'throw' Expr                                -- ParseThrowStatement
                                                              -- (does not consume ';'; a trailing
                                                              --   ';' is parsed as a separate EmptyStmt)

DeclStmt        ::= Decl
ExprStmt        ::= Expr ';'
EmptyStmt       ::= ';'
```

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
| 15 | `,` (only inside argument lists, not a top-level expression operator) | — |

```
Expr            ::= AssignExpr                                 -- ParseExpression
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
                  | '[' Expr ']'                                -- subscript
                  | '.' IDENT                                   -- member access
                  | '++' | '--'                                 -- postfix inc/dec
                  | GenericSpecialization                       -- '<' Type/Expr (',' Type/Expr)* '>'  context-sensitive
AtomExpr        ::= Literal                                    -- parseAtomicExpr (keyword syntax-decl dispatch)
                  | QualifiedName
                  | '(' Expr ')'                                -- parenthesized
                  | '(' Expr (',' Expr)+ ')'                    -- tuple
                  | InitListExpr
                  | KeywordExpr
                  | LambdaExpr
                  | NewExpr
KeywordExpr     ::= 'this'
                  | 'try' Expr
                  | 'no_diff' Expr
                  | ('fwd_diff'|'__fwd_diff') '(' Expr ')'
                  | ('bwd_diff'|'__bwd_diff') '(' Expr ')'
                  | '__apply' '(' Expr ')'                     -- apply-for-backward (experimental)
                  | 'sizeof' '(' Type ')'
                  | 'alignof' '(' Type ')'
                  | 'countof' '(' Expr ')'
                  | '__dispatch_kernel' '(' ArgList ')'
                  | '__getAddress' '(' Expr ')'
                  | '__floatAsInt' '(' Expr ')'
                  | other __-prefixed compiler-internal forms; see keywords-and-builtins.md
LambdaExpr      ::= '(' ParamList? ')' '=>' Expr
                  | IDENT '=>' Expr
NewExpr         ::= 'new' Type ('(' ArgList? ')')?
Literal         ::= INT_LIT | FLOAT_LIT | STRING_LIT | CHAR_LIT
                  | 'true' | 'false'                            -- BoolLiteralExpr
                  | 'nullptr'                                   -- NullPtrLiteralExpr
                  | 'none'                                      -- NoneLiteralExpr
InitListExpr    ::= '{' (Expr (',' Expr)* ','?)? '}'

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
`IntegerLiteralExpr` rather than a dedicated character-literal node.

### `<` disambiguation

`PostfixExpr` may be followed by `<` to start a generic argument
list. The parser uses the strategy described in
[../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md): try to
parse as a generic argument list and check the token after the
matching `>`; if that token is in the "generic-followers" set
(`::`, `.`, `(`, `)`, `[`, `]`, `:`, `,`, `?`, `;`, `==`, `!=`,
`>`, `>>`) treat the `<` as a generic application, otherwise back
out and parse as a comparison. In body-parse mode (function bodies)
the parser also asks the semantic checker whether the preceding
expression resolves to a generic, and uses that as the primary
signal.

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
Attribute       ::= '[' AttributeBody ']'
AttributeBody   ::= AttributeName ('(' ArgList? ')')?
                  | AttributeBody ',' AttributeBody           -- multiple attributes per bracket
AttributeName   ::= IDENT ('::' IDENT)*

ArgList         ::= Expr (',' Expr)*
```

The bracket form `[name(args)]` is identical to a modifier in the
AST representation — `_parseAttribute` constructs `UncheckedAttribute`
nodes that flow through the same `Modifier` chain as keyword
modifiers (see [../ast-reference/modifiers.md](../ast-reference/modifiers.md)).
The attribute name resolves through the same syntax-decl lookup
the parser uses for keyword modifiers, with `attribute_syntax`
declarations supplying the mapping from name to attribute class.
Inside a single bracket, multiple attributes may appear separated by
commas.

## Generics and where-clauses

```
GenericDecl     ::= '__generic' '<' GenericParams '>' Decl     -- parseGenericDecl
                                                              --   ALSO inline form on FuncDecl, StructDecl,
                                                              --   InterfaceDecl, ClassDecl, ExtensionDecl
GenericParams   ::= GenericParam (',' GenericParam)*
GenericParam    ::= IDENT (':' TypeList)?                      -- type parameter
                  | 'let' IDENT ':' Type ('=' Expr)?           -- value parameter
                  | 'each' IDENT (':' TypeList)?               -- pack parameter

WhereClause     ::= ('where' WhereTerm)+                       -- see FuncDecl
WhereTerm       ::= 'optional'? Type ':' Type (',' Type)*      -- conformance constraint(s)
                  | 'optional'? Type '==' Type                 -- equality constraint
                  | 'optional'? 'nonempty' '(' Expr ')'        -- non-empty pack constraint
                  | 'optional'? 'countof' '(' Expr ')' '==' Expr -- variadic pack-count constraint
                  | '__hasDiffTypeInfo' '(' Type ')'           -- differentiable-type-info constraint
                  | Type '(' Type ')' 'implicit'?              -- type-coercion constraint
                                                              --   (maybeParseGenericConstraints builds a
                                                              --   TypeCoercionConstraintDecl: toType '(' fromType ')')
```

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
CoreType        ::= QualifiedName GenericArgs?                 -- _parseSimpleTypeSpec
                  | Type '[' Expr? ']'                          -- array (parsePostfixTypeSuffix)
                  | Type '*'                                    -- pointer-to-T (parsePostfixTypeSuffix)
                  | 'functype' '(' Type (',' Type)* ')' '->' Type
                                                              -- function type (parseFuncTypeExpr)
                  | 'each' Type                                 -- pack type
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

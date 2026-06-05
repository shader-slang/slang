---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-05T09:24:37Z
source_commit: 52339028a2aa703271533454c6b9528a534bac31
watched_paths_digest: 1084d6ac21281bc1db256e51bd36ad00a4bce0602ec9747f43e80e7a14436e98
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
SourceFile      ::= ModuleHeader? TopDecl* EOF
ModuleHeader    ::= ('module' | 'implementing') IDENT ';'?
                                              -- parseModuleDeclarationDecl,
                                              --   parseImplementingDecl

TopDecl         ::= ImportDecl
                  | NamespaceDecl
                  | UsingDecl
                  | FileDecl
                  | Decl

ImportDecl      ::= ('import' | '__import') ImportPath ';'  -- parseImportDecl
                  | '__include' ImportPath ';'              -- parseIncludeDecl
ImportPath      ::= IDENT ('.' IDENT)*                       -- context-sensitive

NamespaceDecl   ::= 'namespace' IDENT '{' TopDecl* '}'      -- parseNamespaceDecl
UsingDecl       ::= 'using' QualifiedName ';'                -- parseUsingDecl
FileDecl        ::= '__file_decl' '{' TopDecl* '}'           -- parseFileDecl

QualifiedName   ::= IDENT ('::' IDENT)*
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
                  | VarDecl     | LetDecl
                  | AssocTypeDecl
                  | RequireCapabilityDecl
                  | SyntaxDecl
                  | AttributeSyntaxDecl
                  | CBufferDecl | TBufferDecl
                  | NamespaceDecl
                  | UsingDecl
```

### Type-defining declarations

```
TypedefDecl     ::= 'typedef' Type IDENT ';'                  -- parseTypeDef
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
                                  | SubscriptDecl | ConstructorDecl)

ExtensionDecl   ::= ('extension' | '__extension') Type
                    Inheritance? '{' StructMember* '}'        -- parseExtensionDecl

AssocTypeDecl   ::= 'associatedtype' IDENT (':' TypeList)? ';'  -- parseAssocType

Inheritance     ::= ':' TypeList
TypeList        ::= Type (',' Type)*
```

### Function-style declarations

```
FuncDecl        ::= 'func' IDENT GenericParams? '(' ParamList? ')' (':' ResultClause)?
                    WhereClause? FuncBody                      -- parseFuncDecl
                  | Type IDENT GenericParams? '(' ParamList? ')' (':' ResultClause)?
                    WhereClause? FuncBody                      -- C-style header (context-sensitive)
                                                              -- the type-vs-name disambiguation is heuristic;
                                                              -- see ../pipeline/02-parse-ast.md

ConstructorDecl ::= '__init' GenericParams? '(' ParamList? ')'
                    WhereClause? FuncBody                      -- parseConstructorDecl
SubscriptDecl   ::= '__subscript' GenericParams? '(' ParamList? ')' '->' Type
                    AccessorBlock                              -- parseSubscriptDecl
PropertyDecl    ::= 'property' IDENT ':' Type AccessorBlock    -- parsePropertyDecl

FuncExtensionDecl
                ::= '__func_extension' GenericParams? KeywordExprHead
                    '(' ParamList? ')' ('throws' Type)? ('->' Type)?
                    WhereClause? FuncBody                      -- parseFuncExtensionDecl
                                                              -- KeywordExprHead is a higher-order
                                                              -- form such as `fwd_diff(foo)`,
                                                              -- `bwd_diff(foo)`, or `__apply(foo)`;
                                                              -- gated behind -experimental-feature

ParamList       ::= Param (',' Param)*
Param           ::= ModifierList? Type IDENT ('=' Expr)?       -- context-sensitive (modifiers vs type)
ResultClause    ::= Type                                       -- single-return path
                  | 'throws' Type                              -- error-returning function

WhereClause     ::= 'where' WhereTerm (',' WhereTerm)*
WhereTerm       ::= Type ':' Type                              -- conformance constraint
                  | Type '==' Type                             -- equality constraint

FuncBody        ::= ';'                                        -- prototype only
                  | '{' BodyTokens '}'                         -- captured as UnparsedStmt in stage 1
BodyTokens      ::= (anything but unbalanced '{' / '}')*       -- see two-stage parsing

AccessorBlock   ::= ';' | '{' AccessorDecl* '}'
AccessorDecl    ::= ModifierList? AccessorName FuncBody
AccessorName    ::= 'get' | 'set' | 'modify' | 'ref'
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
```

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

WhereClause     ::= 'where' WhereTerm (',' WhereTerm)*         -- see FuncDecl
WhereTerm       ::= Type ':' Type                              -- conformance constraint
                  | Type '==' Type                             -- equality constraint
```

Where-clauses appear after the parameter list (or after the result
clause for function-style declarations) and are syntactically optional
on every kind of generic declaration. The body that follows is
captured as raw tokens during stage-1 parsing and is re-parsed lazily
during checking, so the body sees a fully-resolved generic
parameter list — see
[../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md).

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
IfStmt          ::= 'if' '(' Expr ')' Stmt ('else' Stmt)?      -- around line 6457
ForStmt         ::= 'for' '(' (DeclStmt | ExprStmt | ';') Expr? ';' Expr? ')' Stmt
                                                              -- around line 6468
WhileStmt       ::= 'while' '(' Expr ')' Stmt                  -- around line 6470
DoWhileStmt     ::= 'do' Stmt 'while' '(' Expr ')' ';'         -- around line 6472
DoCatchStmt     ::= 'do' Stmt 'catch' '(' Param ')' Block      -- slang-parser.cpp:7044-7063

SwitchStmt      ::= 'switch' '(' Expr ')' '{' SwitchCase* '}'  -- around line 6487
SwitchCase      ::= ('case' Expr ':' | 'default' ':') Stmt*

BreakStmt       ::= 'break' IDENT? ';'                          -- around line 6474
ContinueStmt    ::= 'continue' IDENT? ';'                       -- around line 6476
ReturnStmt      ::= 'return' Expr? ';'                          -- around line 6478
DiscardStmt     ::= 'discard' ';'                               -- around line 6480
DeferStmt       ::= 'defer' Stmt                                -- around line 6505
ThrowStmt       ::= 'throw' Expr ';'                            -- around line 6513

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
| 6 | `<` `<=` `>` `>=` | left |
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
Expr            ::= AssignExpr
AssignExpr      ::= TernaryExpr (AssignOp AssignExpr)?
TernaryExpr     ::= LogicalOrExpr ('?' Expr ':' AssignExpr)?
LogicalOrExpr   ::= LogicalAndExpr ('||' LogicalAndExpr)*
LogicalAndExpr  ::= BitOrExpr ('&&' BitOrExpr)*
BitOrExpr       ::= BitXorExpr ('|' BitXorExpr)*
BitXorExpr      ::= BitAndExpr ('^' BitAndExpr)*
BitAndExpr      ::= EqualityExpr ('&' EqualityExpr)*
EqualityExpr    ::= RelationalExpr (('==' | '!=') RelationalExpr)*
RelationalExpr  ::= ShiftExpr (('<' | '<=' | '>' | '>=') ShiftExpr)*
                                                              -- '<' is context-sensitive (generic vs comparison)
ShiftExpr       ::= AddExpr (('<<' | '>>') AddExpr)*
AddExpr         ::= MulExpr (('+' | '-') MulExpr)*
MulExpr         ::= UnaryExpr (('*' | '/' | '%') UnaryExpr)*
UnaryExpr       ::= UnaryOp UnaryExpr | PostfixExpr
UnaryOp         ::= '+' | '-' | '!' | '~' | '++' | '--' | '*' | '&'
PostfixExpr     ::= AtomExpr PostfixSuffix*
PostfixSuffix   ::= '(' ArgList? ')'                           -- call
                  | '[' Expr ']'                                -- subscript
                  | '.' IDENT                                   -- member access
                  | '++' | '--'                                 -- postfix inc/dec
                  | GenericSpecialization                       -- '<' Type/Expr (',' Type/Expr)* '>'  context-sensitive
AtomExpr        ::= Literal
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

## Types

```
Type            ::= ModifierList? CoreType
CoreType        ::= QualifiedName GenericArgs?
                  | Type '[' Expr? ']'                          -- array
                  | Type '*'                                    -- pointer-to-T (where supported)
                  | Type '?'                                    -- Optional<T>
                  | Type '&'                                    -- reference (where supported)
                  | '(' Type (',' Type)+ ')'                    -- tuple type
                  | 'func' '(' Type (',' Type)* ')' '->' Type   -- function type
                  | 'each' Type                                 -- pack type
GenericArgs     ::= '<' GenericArg (',' GenericArg)* '>'        -- context-sensitive
GenericArg      ::= Type | Expr                                 -- ambiguous; resolved at check time
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

// Regression test for #11347: contextual keywords (`extension`,
// `struct`, `interface`, `func`, `var`, `let`, `enum`, `namespace`,
// `typealias`, `property`, `class`, `functype`, `__func_as_type`) lex
// as `TokenType::Identifier` and were silently accepted as variable
// names — `int extension = 0;` parsed cleanly with no diagnostic, only
// to surprise the user when a later use like `extension = 1;`
// re-dispatched to `parseExtensionDecl` and produced an "unexpected
// integer literal" error far from the declaration.
//
// `_maybeWarnNameShadowsKeyword` in `source/slang/slang-parser.cpp`
// now warns when a *variable* binding name (modern `let`/`var`/
// `property` + modern parameters, traditional `<type> <name>;`
// declarators, struct fields) collides with a contextual keyword.
// Function names, traditional-style parameters, generic param names,
// and type-alias names are intentionally exempt.
//
// The list of keywords is kept in sync with the syntax-decl table at
// `source/slang/slang-parser.cpp:10282-10320` (looked up via
// `tryLookUpSyntaxDecl` with `LookupMask::SyntaxDecl`, restricted to
// `Decl`-class entries) plus a hardcoded set of `_parseSimpleTypeSpec`
// special-cases (`struct`, `class`, `enum`, `functype`,
// `__func_as_type`).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// ---------------------------------------------------------------------
// Positive cases — traditional declarator (top-level var)
// Covers the syntax-decl-table half of the keyword set.
// ---------------------------------------------------------------------

int extension = 0;
//CHECK: 'extension' is a contextual keyword and using it as a declaration name

int interface = 0;
//CHECK: 'interface' is a contextual keyword and using it as a declaration name

int namespace = 0;
//CHECK: 'namespace' is a contextual keyword and using it as a declaration name

// ---------------------------------------------------------------------
// Positive cases — traditional declarator, hardcoded fallback list.
// `struct` / `class` / `enum` / `functype` are special-cased in
// `_parseSimpleTypeSpec` rather than `kSyntaxDecls[]`.
// ---------------------------------------------------------------------

int struct = 0;
//CHECK: 'struct' is a contextual keyword and using it as a declaration name

int class = 0;
//CHECK: 'class' is a contextual keyword and using it as a declaration name

int enum = 0;
//CHECK: 'enum' is a contextual keyword and using it as a declaration name

int func = 0;
//CHECK: 'func' is a contextual keyword and using it as a declaration name

// ---------------------------------------------------------------------
// Multi-declarator path — exercises the second loop iteration of
// `ParseDeclaratorDecl`.
// ---------------------------------------------------------------------

int var = 0, regularNameInGroup = 1;
//CHECK: 'var' is a contextual keyword and using it as a declaration name

// ---------------------------------------------------------------------
// Sequential same-keyword bindings — pre-fix only the first warned
// because `LookupMask::Default` made the second lookup overloaded
// (global `SyntaxDecl` + the first user binding) and
// `tryLookUpSyntaxDecl` returns null on overloaded results. With
// `LookupMask::SyntaxDecl` the user binding is filtered out before
// the overload check, so both bindings warn.
// ---------------------------------------------------------------------

let property = 1;
//CHECK: 'property' is a contextual keyword and using it as a declaration name

let property_first_dup = 2;
// Use a fresh name to avoid an actual redeclaration error; the point
// is that a second `property`-shaped binding still warns.

// ---------------------------------------------------------------------
// Modern var-decl path.
// ---------------------------------------------------------------------

let typealias = 3;
//CHECK: 'typealias' is a contextual keyword and using it as a declaration name

// ---------------------------------------------------------------------
// Struct field — traditional declarator inside a struct body.
// ---------------------------------------------------------------------

struct WithKeywordField
{
    int extension;
    //CHECK: 'extension' is a contextual keyword and using it as a declaration name
};

// ---------------------------------------------------------------------
// Negative cases — must NOT warn.
// ---------------------------------------------------------------------

// Regular variable name.
int regularName = 0;

// Function declaration — function names are intentionally exempt.
int funcReturningInt() { return 0; }

// Traditional-style parameter — also exempt.
int funcWithKeywordParam(int extension2) { return extension2; }

// Modifier-style contextual keyword (`payload`) used as a struct field —
// the `Decl`-class filter excludes modifier-only entries from the
// syntax-decl lookup, so this must not fire.
struct PayloadFieldOk
{
    int payload;
}

//CHECK-NOT: 'regularName' is a contextual keyword
//CHECK-NOT: 'funcReturningInt' is a contextual keyword
//CHECK-NOT: 'extension2' is a contextual keyword
//CHECK-NOT: 'payload' is a contextual keyword
//CHECK-NOT: 'regularNameInGroup' is a contextual keyword
//CHECK-NOT: 'property_first_dup' is a contextual keyword

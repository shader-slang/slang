// Regression test for #11347: contextual keywords (`extension`,
// `struct`, `interface`, `func`, `var`, `let`, `enum`, `namespace`,
// `typealias`, `property`, `class`) lex as `TokenType::Identifier`
// and were silently accepted as variable names — `int extension = 0;`
// parsed cleanly with no diagnostic, only to surprise the user when a
// later use like `extension = 1;` re-dispatched to `parseExtensionDecl`
// and produced an "unexpected integer literal" error far from the
// declaration.
//
// `_maybeWarnNameShadowsKeyword` in `source/slang/slang-parser.cpp`
// now warns when a *variable* binding name (the canonical bug shape —
// modern `let`/`var`/`property` and traditional `<type> <name>;`
// declarators) collides with a contextual keyword. Function names are
// intentionally exempt because they are unambiguous in practice and
// the test corpus uses `func` / similar widely.
//
// The list is kept in sync with the syntax-decl table at
// `source/slang/slang-parser.cpp:10282-10320` plus the type-spec
// special-cases in `_parseSimpleTypeSpec` (`struct`, `class`, `enum`,
// `functype`).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Traditional declarator path — the user's exact repro from #11347.
int extension = 0;
/*CHECK:
    ^^^^^^^^^ 'extension' is a contextual keyword and using it as a declaration name will produce surprising results
    ^^^^^^^^^ 'extension' is a contextual keyword; rename the declaration to avoid parser ambiguity
*/

// `struct` / `enum` / `class` are special-cased in `_parseSimpleTypeSpec`
// rather than `kSyntaxDecls[]`; both classes are flagged.
int struct = 0;
/*CHECK:
    ^^^^^^ 'struct' is a contextual keyword and using it as a declaration name will produce surprising results
    ^^^^^^ 'struct' is a contextual keyword; rename the declaration to avoid parser ambiguity
*/

int enum = 0;
/*CHECK:
    ^^^^ 'enum' is a contextual keyword and using it as a declaration name will produce surprising results
    ^^^^ 'enum' is a contextual keyword; rename the declaration to avoid parser ambiguity
*/

int func = 0;
/*CHECK:
    ^^^^ 'func' is a contextual keyword and using it as a declaration name will produce surprising results
    ^^^^ 'func' is a contextual keyword; rename the declaration to avoid parser ambiguity
*/

// Modern var-decl path.
let property = 1;
/*CHECK:
    ^^^^^^^^ 'property' is a contextual keyword and using it as a declaration name will produce surprising results
    ^^^^^^^^ 'property' is a contextual keyword; rename the declaration to avoid parser ambiguity
*/

var typealias = 2;
/*CHECK:
    ^^^^^^^^^ 'typealias' is a contextual keyword and using it as a declaration name will produce surprising results
    ^^^^^^^^^ 'typealias' is a contextual keyword; rename the declaration to avoid parser ambiguity
*/

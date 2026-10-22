//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-unscoped-enum

// With `-unscoped-enum`, each case adds a `static const` alias to the enclosing scope. A case
// with the same name as its own enum type conflicts with that type, as `struct x {}` and
// `int x;` do (shader-slang/slang#13876).

enum Mode
/*CHECK:
     ^^^^ see previous declaration of 'Mode'
*/
{
    Mode,
/*CHECK:
    ^^^^ conflicting declaration
    ^^^^ declaration of 'Mode' conflicts with existing declaration
*/
    Other,
};

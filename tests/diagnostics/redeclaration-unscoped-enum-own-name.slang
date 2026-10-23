//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-unscoped-enum -std 2018
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-unscoped-enum -std 2026
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-unscoped-enum -std 202c

// With `-unscoped-enum`, each case adds a `static const` alias to the enclosing scope. A case
// with the same name as its own enum gets a warning in every language version, and both the
// type and the value remain usable (shader-slang/slang#13876).

enum Mode
{
    Mode,
/*CHECK:
    ^^^^ warning
    ^^^^ case 'Mode' has the same name as its unscoped enum
*/
    Other,
};

int takesMode(Mode m)
{
    return (int)m;
}

int useTypeAndValue()
{
    Mode m = Other;
    return takesMode(m) + (int)Mode;
}

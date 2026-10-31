//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/diagnostics/hlsl-for-loop-variable-scope.slang -no-codegen

// In HLSL, as in C++ and DXC, the variable declared in a `for` initializer is scoped to the
// loop, so code after the loop cannot use it (shader-slang/slang#14099).

int readAfterLoop()
{
    for (int i = 0; i < 2; ++i) {}
    return i;
/*CHECK:
           ^ error E30015
           ^ undefined identifier 'i'.
*/
}

int continueCountingInLaterLoop()
{
    int value = 0;
    for (int i = 0; i < 2; ++i) { value += i; }
    for (; i < 4; ++i) { value += i; }
/*CHECK:
           ^ error E30015
           ^ undefined identifier 'i'.
                    ^ error E30015
                    ^ undefined identifier 'i'.
                                  ^ error E30015
                                  ^ undefined identifier 'i'.
*/
    return value;
}

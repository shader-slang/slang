//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen -warnings-disable 30081

// The Slang dialect rejects a declaration used as a condition with one dedicated
// diagnostic (its message plus the explanatory span annotated below), instead of a
// cascade of unrelated parse errors. The declaration is still parsed for error
// recovery. The 30081 warning is disabled: the test's `int` conditions convert to
// `bool` by design, and the recovery parse reports that conversion like any other.

void main()
{
    int c = 1;
    if (int count = c)
    /*CHECK:
        ^^^ a declaration cannot be used as a condition
        ^^^ a variable declaration can only be used as the condition of an `if`, `while`, or `do`-`while` statement in the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`).
    */
    {}
    while (int count = c)
    /*CHECK:
           ^^^ a declaration cannot be used as a condition
           ^^^ a variable declaration can only be used as the condition of an `if`, `while`, or `do`-`while` statement in the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`).
    */
    {}
    do
    {
    } while (int count = c);
    /*CHECK:
             ^^^ a declaration cannot be used as a condition
             ^^^ a variable declaration can only be used as the condition of an `if`, `while`, or `do`-`while` statement in the HLSL-flavored dialect (use a `.hlsl` file or pass `-lang hlsl`).
    */
}

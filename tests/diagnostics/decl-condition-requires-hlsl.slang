//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen -warnings-disable 30081

// The Slang dialect rejects a declaration used as a condition with a dedicated
// diagnostic, instead of the generic unexpected-token cascade. The declaration is
// still parsed for error recovery, so there is exactly one clear message per
// condition.

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

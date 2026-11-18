//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// The Slang dialect rejects a declaration used as a condition with a dedicated
// diagnostic, instead of the generic unexpected-token cascade.

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

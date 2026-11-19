//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/hlsl/decl-condition-errors.slang -no-codegen -warnings-disable 30081

// Error cases for HLSL-dialect declaration conditions: storage-class modifiers
// contradict the re-initialize-before-every-test rule, more than one declarator is
// rejected like C++, a condition value must be convertible to `bool`, and the
// `do`-`while` condition variable is invisible in the body (the body runs before
// the condition). The 30081 warning is disabled: the `int` conditions convert to
// `bool` by design.

void main()
{
    int c = 1;

    while (static int i = c)
    /*CHECK:
           ^^^^^^ a condition declaration cannot have a storage-class modifier
           ^^^^^^ a condition variable is re-initialized before every evaluation of the condition, so storage-class modifiers like `static` and `groupshared` cannot apply to it.
    */
    {
        break;
    }

    if (int a = 1, b = 2)
    /*CHECK:
                 ^ a condition declaration allows only one declarator
                 ^ as in C++, a declaration used as a condition can declare a single variable; declare additional variables before the statement.
    */
    {
    }

    float4 v0 = float4(1, 1, 1, 1);
    if (float4 v = v0)
    /*CHECK:
               ^ type mismatch in expression
               ^ expected an expression of type 'bool', got 'vector<float,4>'
    */
    {
    }

    // The `do`-`while` condition variable is visible in the condition only; the body
    // runs before the condition is even parsed, so the name is undefined in the body.
    int sum = 0;
    do
    {
        sum += cond;
        /*CHECK:
               ^^^^ undefined identifier
               ^^^^ undefined identifier 'cond'.
        */
    }
    while (int cond = 0);
}

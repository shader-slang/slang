//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang hlsl tests/hlsl/decl-condition-errors.slang -no-codegen -warnings-disable 30081

// Error cases for declaration conditions: a condition value must be convertible to
// `bool`, and the declared variable is invisible in the body (the body
// is parsed before the condition, so the name is undefined there). The 30081 warning
// is disabled: the `int` conditions convert to `bool` by design.

void main()
{
    float4 v0 = float4(1, 1, 1, 1);
    if (float4 v = v0)
    /*CHECK:
               ^ type mismatch in expression
               ^ expected an expression of type 'bool', got 'vector<float,4>'
    */
    {
    }

    // The `do`-`while` declared variable is visible in the condition only; the body
    // is parsed before the condition, so the name is undefined in the body.
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

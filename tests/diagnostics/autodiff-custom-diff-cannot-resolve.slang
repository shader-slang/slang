//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test for error 31148 (cannot resolve derivative function).
// Referencing a nonexistent function in the derivative attribute triggers this error.
// Note: Using [BackwardDerivative] instead of [ForwardDerivative] because the latter
// triggers a compiler crash (segfault) when the derivative function is unresolved.

[BackwardDerivative(nonexistent_bwd)]
/*CHECK:
                    ^^^^^^^^^^^^^^^ undefined identifier
                    ^^^^^^^^^^^^^^^ undefined identifier 'nonexistent_bwd'.
 ^^^^^^^^^^^^^^^^^^ cannot resolve derivative function
 ^^^^^^^^^^^^^^^^^^ cannot resolve the custom derivative function
*/
float f(float x)
{
    return x * x;
}

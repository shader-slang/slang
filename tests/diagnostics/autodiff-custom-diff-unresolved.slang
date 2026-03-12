//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Simple check to see that the compiler returns an error message if the
// custom derivative definition points to an undefined function.
//

void __d_f(float x, float.Differential dout)
{
}

[BackwardDerivative(__d_g)]
/*CHECK:
                    ^^^^^ undefined identifier
                    ^^^^^ undefined identifier '__d_g'.
 ^^^^^^^^^^^^^^^^^^ cannot resolve derivative function
 ^^^^^^^^^^^^^^^^^^ cannot resolve the custom derivative function
*/
float f(float x)
{
    return x * x;
}

float main(float x)
{
    DifferentialPair<float> dpx = diffPair(x, 1.f);
    bwd_diff(f)(dpx, 1.f);
}
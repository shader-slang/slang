//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[ForwardDifferentiable]
float f(float x)
{
    float result = 0.0;
    int j = 0;
    for (int i = 0; i < 5; j++)
/*CHECK:
                           ^ for loop modifies wrong variable
                           ^ the for loop initializes and checks variable 'i' but the side effect expression is modifying 'j'.
    ^^^ loop in differentiable function needs attributes
    ^^^ loops inside a differentiable function need to provide either '[MaxIters(n)]' or '[ForceUnroll]' attribute.
*/
    {
        result += x;
    }
    return result;
}

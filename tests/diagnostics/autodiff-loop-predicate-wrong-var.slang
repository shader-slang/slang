//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[ForwardDifferentiable]
float f(float x)
{
    float result = 0.0;
    int j = 0;
    for (int i = 0; j < 5; i++)
/*CHECK:
                      ^ for loop predicate checks wrong variable
                      ^ the for loop initializes and modifies variable 'i' but the predicate expression is checking 'j'.
    ^^^ loop in differentiable function needs attributes
    ^^^ loops inside a differentiable function need to provide either '[MaxIters(n)]' or '[ForceUnroll]' attribute.
*/
    {
        result += x;
    }
    return result;
}

//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive):

float nonDiff(float x)
{
    return x;
}

[ForwardDifferentiable]
float f(float x)
{
    return x * x;
}

[BackwardDifferentiable]
float g(float x)
{
    float val = f(x + 1); // Error: f must also be backward-differentiable
//diag:          ^ derivative cannot be propagated through call to non-backward-differentiable function `f`, use 'no_diff' to clarify intention.
    return val;
}

[BackwardDifferentiable]
float h(float x)
{
    float val = 0;
    // no diagnostic by clarifying intention.
    val = no_diff(f(x + 1));

    // error: dynamic loop without [MaxIters] or [ForceUnroll]
    for (int i = 0; i < (int)x; i++)
/*diag:
    ^^^ loop in differentiable function needs attributes
    ^^^ loops inside a differentiable function need to provide either '[MaxIters(n)]' or '[ForceUnroll]' attribute.
*/
    {
        no_diff debugBreak();
    }

    return val;
}

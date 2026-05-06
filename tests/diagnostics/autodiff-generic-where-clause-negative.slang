// Test that differentiating a generic function without IDifferentiable constraint produces an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

float genericFunc<T>(T val, float x)
{
    return x * x;
}

[ForwardDifferentiable]
float caller(float x)
{
    return genericFunc<float>(x, x);
//CHECK:                     ^ derivative cannot be propagated through call to non-forward-differentiable function `genericFunc`, use 'no_diff' to clarify intention.
//CHECK:                     ^ derivative cannot be propagated through non-differentiable call
}

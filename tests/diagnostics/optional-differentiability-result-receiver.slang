//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

[Differentiable]
T identity<T>(T value) where optional T : IDifferentiable
//CHECK: a differentiable function's result must not have optional differentiability
{
    return value;
}

extension<T> T where optional T : IDifferentiable
{
    [Differentiable]
    float evaluate(float value)
    //CHECK: a differentiable function's receiver must not have optional differentiability
    {
        return value;
    }
}

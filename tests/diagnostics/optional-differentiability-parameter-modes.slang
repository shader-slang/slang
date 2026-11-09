//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

[Differentiable]
float withInOut<T>(inout T value) where optional T : IDifferentiable
//CHECK: a function with optionally differentiable inputs must use simple 'in' parameters
{
    return 1.0;
}

[Differentiable]
float withOut<T>(out T value) where optional T : IDifferentiable
//CHECK: a function with optionally differentiable inputs must use simple 'in' parameters
{
    value = {};
    return 1.0;
}

[Differentiable]
float withRef<T>(__ref T value) where optional T : IDifferentiable
//CHECK: a function with optionally differentiable inputs must use simple 'in' parameters
{
    return 1.0;
}

[Differentiable]
float withOtherOutput<T>(T value, out float result) where optional T : IDifferentiable
//CHECK: a function with optionally differentiable inputs must use simple 'in' parameters
{
    result = 0.0;
    return 1.0;
}

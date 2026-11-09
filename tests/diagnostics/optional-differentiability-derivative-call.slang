//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

[Differentiable]
float evaluate<T>(T value) where optional T : IDifferentiable
{
    return 1.0;
}

void requestForward<T>() where optional T : IDifferentiable
{
    let derivative = fwd_diff(evaluate<T>);
    //CHECK: cannot use 'fwd_diff' or 'bwd_diff' before optional differentiability is resolved
}

void requestBackward<T>() where optional T : IDifferentiable
{
    let derivative = bwd_diff(evaluate<T>);
    //CHECK: cannot use 'fwd_diff' or 'bwd_diff' before optional differentiability is resolved
}

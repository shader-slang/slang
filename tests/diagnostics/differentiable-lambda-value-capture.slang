//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

float evaluate(float scale, float value)
{
    let function = (float v)~ => scale * v;
    //CHECK: cannot capture differentiable value type 'float' in a differentiable lambda
    return function(value);
}

// Test higher-order forward differentiation of a generic function
// where the generic type parameter is used as the function parameter type.

//TEST:INTERPRET(filecheck=CHECK):

[ForwardDifferentiable]
T genericPow4<T : IDifferentiable & IFloat>(T x)
{
    return x * x * x * x;
}

// First derivative wrapper.
[ForwardDifferentiable]
float dGenericPow4(float x)
{
    return fwd_diff(genericPow4<float>)(DifferentialPair<float>(x, 1.0)).d;
}

void main()
{
    // f(x) = x^4
    // f'(x) = 4*x^3
    // f''(x) = 12*x^2
    // f''(2) = 12 * 4 = 48
    let result = fwd_diff(dGenericPow4)(DifferentialPair<float>(2.0, 1.0)).d;
    // CHECK: 48.0
    printf("%f\n", result);
}

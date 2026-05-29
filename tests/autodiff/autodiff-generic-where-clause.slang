// Test differentiating a generic function with a where clause requiring IDifferentiable.
// The generic-typed parameter `val` participates in the computation.

//TEST:INTERPRET(filecheck=CHECK):

[ForwardDifferentiable]
T genericCalc<T : IDifferentiable & IFloat>(T val, T x)
{
    return val * x * x + x;
}

void main()
{
    // genericCalc(val, x) = val*x^2 + x
    // d/dx genericCalc = 2*val*x + 1
    // At val = 2.0, x = 3.0: derivative = 2*2*3 + 1 = 13.0
    let result = fwd_diff(genericCalc<float>)(
        DifferentialPair<float>(2.0, 0.0),
        DifferentialPair<float>(3.0, 1.0));
    // CHECK: 13.0
    printf("%f\n", result.d);

    // d/dval genericCalc = x^2
    // At val = 2.0, x = 3.0: derivative w.r.t. val = 9.0
    let result2 = fwd_diff(genericCalc<float>)(
        DifferentialPair<float>(2.0, 1.0),
        DifferentialPair<float>(3.0, 0.0));
    // CHECK: 9.0
    printf("%f\n", result2.d);
}

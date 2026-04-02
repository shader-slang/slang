// Test differentiating a generic function with a where clause requiring IDifferentiable.

//TEST:INTERPRET(filecheck=CHECK):

[ForwardDifferentiable]
float genericCalc<T : IDifferentiable>(T val, float x)
{
    return x * x + x;
}

void main()
{
    // genericCalc(val, x) = x^2 + x
    // d/dx genericCalc = 2*x + 1
    // At x = 3.0: derivative = 7.0
    let result = fwd_diff(genericCalc<float>)(
        DifferentialPair<float>(0.0, 0.0),
        DifferentialPair<float>(3.0, 1.0));
    // CHECK: 7.0
    printf("%f\n", result.d);

    // At x = 5.0: derivative = 11.0
    let result2 = fwd_diff(genericCalc<float>)(
        DifferentialPair<float>(0.0, 0.0),
        DifferentialPair<float>(5.0, 1.0));
    // CHECK: 11.0
    printf("%f\n", result2.d);
}

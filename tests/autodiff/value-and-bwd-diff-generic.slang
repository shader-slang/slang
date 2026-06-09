// value_and_bwd_diff generic specialization (Q4): mirrors bwd_diff specialization.

//TEST:INTERPRET(filecheck=CHECK):

[BackwardDifferentiable]
T scaledSquare<T : IDifferentiable & IFloat>(T val, T x)
{
    return val * x * x;
}

void main()
{
    DifferentialPair<float> dpVal = DifferentialPair<float>(3.0, 0.0);
    DifferentialPair<float> dpX = DifferentialPair<float>(2.0, 0.0);

    // Surfaced primal: 3 * 4 = 12.0
    let y = value_and_bwd_diff(scaledSquare<float>)(dpVal, dpX, 1.0);

    // CHECK: y=12.0
    printf("y=%f\n", y);

    // d/dval scaledSquare = x*x = 4
    // CHECK: dval=4.0
    printf("dval=%f\n", dpVal.d);

    // d/dx scaledSquare = 2*val*x = 12
    // CHECK: dx=12.0
    printf("dx=%f\n", dpX.d);
}

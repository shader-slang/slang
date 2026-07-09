// Test higher-order differentiation through a struct with a differentiable method.

//TEST:INTERPRET(filecheck=CHECK):

struct Quadratic : IDifferentiable
{
    typealias Differential = Quadratic;
    float a;
}

[ForwardDifferentiable]
float evalQuadratic(Quadratic q, float x)
{
    return q.a * x * x;
}

[ForwardDifferentiable]
float compute(float x)
{
    Quadratic q = { 2.0 };
    return evalQuadratic(q, x);
}

[ForwardDifferentiable]
float dCompute(float x)
{
    return fwd_diff(compute)(DifferentialPair<float>(x, 1.0)).d;
}

void main()
{
    // f(x) = 2*x^2
    // f'(x) = 4*x
    // f'(3) = 12
    let result = fwd_diff(compute)(DifferentialPair<float>(3.0, 1.0));
    // CHECK: 12.0
    printf("%f\n", result.d);

    // f''(x) = 4
    let result2 = fwd_diff(dCompute)(DifferentialPair<float>(3.0, 1.0)).d;
    // CHECK: 4.0
    printf("%f\n", result2);
}

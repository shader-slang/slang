// Test providing a custom forward derivative via an extension.

//TEST:INTERPRET(filecheck=CHECK):

struct MyStruct
{
    float x;

    float compute(float v)
    {
        return v * v;
    }
}

extension MyStruct
{
    [ForwardDerivativeOf(compute)]
    DifferentialPair<float> diff_compute(DifferentialPair<float> v)
    {
        // Custom derivative: returns 3*v*dv instead of the real 2*v*dv
        // to prove the custom derivative is being used.
        return diffPair(v.p * v.p, 3.0 * v.p * v.d);
    }
}

[ForwardDifferentiable]
float test(MyStruct obj, float v)
{
    return obj.compute(v);
}

void main()
{
    MyStruct s = { 1.0 };
    // If custom derivative is used: d/dv(v^2) at v=4 with dv=1 => 3*4*1 = 12
    // If real derivative were used: 2*4*1 = 8
    let result = fwd_diff(test)(s, DifferentialPair<float>(4.0, 1.0));
    // CHECK: 12.0
    printf("%f\n", result.d);
}

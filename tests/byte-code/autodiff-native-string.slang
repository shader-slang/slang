//TEST:INTERPRET(filecheck=BC): -disasm
//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
float square(float x, float y)
{
    let x2 = x * x;
    let y2 = debugGrad("y2", y * y);
    return x2 + y2;
}

float debugGrad(NativeString label, float x)
{
    return x;
}

[BackwardDerivativeOf(debugGrad)]
void debugGradBwd(NativeString label, inout DifferentialPair<float> x, float dOut)
{
    printf("Gradient of %s is %f\n", label, dOut);
    x = diffPair(x.p, dOut);
}

int main()
{
    var x = diffPair(3.0, 0.0);
    var y = diffPair(4.0, 0.0);
    bwd_diff(square)(x, y, 1.0f);
    printf("dL/dx: %f, dL/dy: %f\n", x.d, y.d);
    return 0;
}

// BC: func s_bwdProp_square
// BC: call {{.*}}, s_bwdProp_debugGrad
// CHECK: Gradient of y2 is 1.000000
// CHECK: dL/dx: 6.000000, dL/dy: 8.000000

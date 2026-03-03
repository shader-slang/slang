//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

float someNoDiffFunc(float x, no_diff float y)
{
    return x * x + y * y;
}

// Previously, when we call a no-diff function side a differntiable function, we will have to use no_diff to tell compiler that this is intended.
// However, if the parameter is just a constant, there is no need to use no_diff, because constant won't carry any derivative information.
// Therefore, this test is to check we won't report any error when the parameter is a constant in this case.
[Differentiable]
float eval(float x)
{
    // CHECK-NOT: ([[# @LINE+1]]): error 41020
    return exp(x) - someNoDiffFunc(1.0f, x);
}

[Differentiable]
float eval1(float x)
{
    // CHECK: ([[# @LINE+1]]): error 41020
    return exp(x) - someNoDiffFunc(x, 1.0);
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint id : SV_DispatchThreadID)
{
    var x = diffPair(2.0f);
    bwd_diff(eval)(x, 1.0f);

    output[0] = x.d;

    var x1 = diffPair(2.0f);
    bwd_diff(eval1)(x1, 1.0f);
    output[1] = x1.d;
}


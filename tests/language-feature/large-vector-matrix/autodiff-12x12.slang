// A differentiable function over a 12-vector and a 12x12 matrix: forward and backward mode
// derivatives go through the same lowering as the primal code.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typealias Mat12 = matrix<float, 12, 12>;
typealias Vec12 = vector<float, 12>;

// e(A, x) = x^T A x / 2 + sum(max(x, 3))
[Differentiable]
float energy(Mat12 a, Vec12 x)
{
    return 0.5 * dot(x, mul(a, x)) + dot(max(x, Vec12(3.0)), Vec12(1.0));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    Mat12 a;
    Vec12 x;
    for (int i = 0; i < 12; i++)
    {
        x[i] = float(i);
        for (int j = 0; j < 12; j++)
            a[i][j] = float(min(i, j)); // symmetric
    }

    // Forward mode along x[5]: d/dx5 = (A x)[5] + (x5 > 3 ? 1 : 0).
    Vec12 dx = Vec12(0.0);
    dx[5] = 1.0;
    let fwd = fwd_diff(energy)(diffPair(a, Mat12(0.0)), diffPair(x, dx));
    // CHECK: {{^}}311.000000
    outputBuffer[0] = fwd.d;

    // Backward mode: dE/dx = A x + step, dE/dA = x x^T / 2.
    var dpA = diffPair(a);
    var dpX = diffPair(x);
    bwd_diff(energy)(dpA, dpX, 1.0);
    // CHECK-NEXT: {{^}}311.000000
    outputBuffer[1] = dpX.d[5];
    // CHECK-NEXT: {{^}}0.000000
    outputBuffer[2] = dpX.d[0];
    // CHECK-NEXT: {{^}}3.000000
    outputBuffer[3] = dpA.d[2][3];
    // CHECK-NEXT: {{^}}60.500000
    outputBuffer[4] = dpA.d[11][11];
    // CHECK-NEXT: {{^}}13481.000000
    outputBuffer[5] = fwd.p;
}

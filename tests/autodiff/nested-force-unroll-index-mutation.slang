//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float innerLoop(float4 values, uint2 idx)
{
    float result = 0.0;
    [ForceUnroll]
    for (int j = 0; j < 4; j++)
    {
        result += values[j] * float(idx.y + 1);
        idx.y += 1;
    }
    return result;
}

[Differentiable]
float outerLoop(float4x4 mat)
{
    float result = 0.0;
    uint2 idx = uint2(0, 0);
    [ForceUnroll]
    for (int i = 0; i < 4; i++)
    {
        result += innerLoop(mat[i], idx);
        idx.y += 4;
    }
    return result;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    float4x4 mat = float4x4(
        1.0, 2.0, 3.0, 4.0,
        5.0, 6.0, 7.0, 8.0,
        9.0, 10.0, 11.0, 12.0,
        13.0, 14.0, 15.0, 16.0
    );

    DifferentialPair<float4x4> dp = diffPair(mat, float4x4(0.0));
    __bwd_diff(outerLoop)(dp, 1.0);

    // d(result)/d(mat[i][j]) = float(offset_at_iter_i + j + 1)
    // Row 0: idx.y starts at 0, so gradients for [0][0..3] are 1, 2, 3, 4
    // Row 1: idx.y starts at 4, so gradients for [1][0..3] are 5, 6, 7, 8
    // Row 2: idx.y starts at 8, so gradients for [2][0..3] are 9, 10, 11, 12
    // Row 3: idx.y starts at 12, so gradients for [3][0..3] are 13, 14, 15, 16
    outputBuffer[0] = dp.d[0][0]; // CHECK: 1.0
    outputBuffer[1] = dp.d[1][0]; // CHECK: 5.0
    outputBuffer[2] = dp.d[2][0]; // CHECK: 9.0
    outputBuffer[3] = dp.d[3][0]; // CHECK: 13.0
}

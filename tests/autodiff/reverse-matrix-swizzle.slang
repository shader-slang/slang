//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

// Test that backward differentiation works correctly when a matrix is
// accessed exclusively through _XY member swizzles (e.g. _11, _12, _21, _22),
// without any direct use of vector types.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float inverse2x2Sum(float2x2 matrix)
{
    // Compute inverse via _XY swizzle access only.
    const float determinant = matrix._11 * matrix._22 - matrix._12 * matrix._21;
    const float reciprocalDeterminant = 1.0 / determinant;

    float2x2 inverseMatrix;
    inverseMatrix._11 = matrix._22 * reciprocalDeterminant;
    inverseMatrix._12 = -matrix._12 * reciprocalDeterminant;
    inverseMatrix._21 = -matrix._21 * reciprocalDeterminant;
    inverseMatrix._22 = matrix._11 * reciprocalDeterminant;

    // Return sum of all inverse entries, using _XY swizzles to read the result.
    return inverseMatrix._11 + inverseMatrix._12 + inverseMatrix._21 + inverseMatrix._22;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // M = [[1, 2], [3, 5]], det = 1*5 - 2*3 = -1
    // inverse = [[-5, 2], [3, -1]], sum = -1
    float2x2 m = float2x2(1.0, 2.0, 3.0, 5.0);
    DifferentialPair<float2x2> dpM = diffPair(m);

    __bwd_diff(inverse2x2Sum)(dpM, 1.0);

    // Expected gradients: dL/da = -6, dL/db = 4, dL/dc = 3, dL/dd = -2
    outputBuffer[0] = dpM.d._11; // CHECK: -6.0
    outputBuffer[1] = dpM.d._12; // CHECK: 4.0
    outputBuffer[2] = dpM.d._21; // CHECK: 3.0
    outputBuffer[3] = dpM.d._22; // CHECK: -2.0

    // Also verify the forward value is correct (sum of inverse = -1).
    outputBuffer[4] = inverse2x2Sum(m); // CHECK: -1.0
}

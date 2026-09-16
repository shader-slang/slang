//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-mtl -compute -shaderobj

// Verify that min and max accept scalar, vector, and matrix operands when generic code knows only
// that the element type belongs to the sealed builtin arithmetic domain.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

T genericMinimum<T : __BuiltinArithmeticType>(T left, T right)
{
    return min(left, right);
}

T genericMaximum<T : __BuiltinArithmeticType>(T left, T right)
{
    return max(left, right);
}

vector<T, N> genericMinimum<T : __BuiltinArithmeticType, let N : int>(
    vector<T, N> left,
    vector<T, N> right)
{
    return min(left, right);
}

vector<T, N> genericMaximum<T : __BuiltinArithmeticType, let N : int>(
    vector<T, N> left,
    vector<T, N> right)
{
    return max(left, right);
}

matrix<T, R, C> genericMinimum<T : __BuiltinArithmeticType, let R : int, let C : int>(
    matrix<T, R, C> left,
    matrix<T, R, C> right)
{
    return min(left, right);
}

matrix<T, R, C> genericMaximum<T : __BuiltinArithmeticType, let R : int, let C : int>(
    matrix<T, R, C> left,
    matrix<T, R, C> right)
{
    return max(left, right);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int3 intMinimum = genericMinimum(int3(5, -2, 9), int3(3, 7, 1));
    int3 intMaximum = genericMaximum(int3(5, -2, 9), int3(3, 7, 1));
    outputBuffer[0] = intMinimum.x;
    outputBuffer[1] = intMinimum.y;
    outputBuffer[2] = intMinimum.z;
    outputBuffer[3] = intMaximum.x;
    outputBuffer[4] = intMaximum.y;
    outputBuffer[5] = intMaximum.z;

    uint2 uintMinimum = genericMinimum(uint2(8, 20), uint2(12, 4));
    uint2 uintMaximum = genericMaximum(uint2(8, 20), uint2(12, 4));
    outputBuffer[6] = int(uintMinimum.x);
    outputBuffer[7] = int(uintMinimum.y);
    outputBuffer[8] = int(uintMaximum.x);
    outputBuffer[9] = int(uintMaximum.y);

    float4 floatMinimum = genericMinimum(float4(2.5, -1.0, 7.0, 4.0), float4(4.0, 3.0, 2.0, 4.0));
    float4 floatMaximum = genericMaximum(float4(2.5, -1.0, 7.0, 4.0), float4(4.0, 3.0, 2.0, 4.0));
    outputBuffer[10] = int(floatMinimum.x * 10.0);
    outputBuffer[11] = int(floatMinimum.y * 10.0);
    outputBuffer[12] = int(floatMinimum.z * 10.0);
    outputBuffer[13] = int(floatMinimum.w * 10.0);
    outputBuffer[14] = int(floatMaximum.x * 10.0);
    outputBuffer[15] = int(floatMaximum.y * 10.0);
    outputBuffer[16] = int(floatMaximum.z * 10.0);
    outputBuffer[17] = int(floatMaximum.w * 10.0);

    outputBuffer[18] = genericMinimum(-3, 5);
    outputBuffer[19] = genericMaximum(-3, 5);
    outputBuffer[20] = int(genericMinimum(1.5, 2.5) * 10.0);
    outputBuffer[21] = int(genericMaximum(1.5, 2.5) * 10.0);

    int2x2 intMatrixMinimum = genericMinimum(
        int2x2(5, -2, 9, 4), int2x2(3, 7, 1, 6));
    int2x2 intMatrixMaximum = genericMaximum(
        int2x2(5, -2, 9, 4), int2x2(3, 7, 1, 6));
    outputBuffer[22] = intMatrixMinimum[0][0];
    outputBuffer[23] = intMatrixMinimum[0][1];
    outputBuffer[24] = intMatrixMinimum[1][0];
    outputBuffer[25] = intMatrixMinimum[1][1];
    outputBuffer[26] = intMatrixMaximum[0][0];
    outputBuffer[27] = intMatrixMaximum[0][1];
    outputBuffer[28] = intMatrixMaximum[1][0];
    outputBuffer[29] = intMatrixMaximum[1][1];

    uint2x2 uintMatrixMinimum = genericMinimum(
        uint2x2(8, 20, 6, 11), uint2x2(12, 4, 9, 3));
    uint2x2 uintMatrixMaximum = genericMaximum(
        uint2x2(8, 20, 6, 11), uint2x2(12, 4, 9, 3));
    outputBuffer[30] = int(uintMatrixMinimum[0][0]);
    outputBuffer[31] = int(uintMatrixMinimum[0][1]);
    outputBuffer[32] = int(uintMatrixMinimum[1][0]);
    outputBuffer[33] = int(uintMatrixMinimum[1][1]);
    outputBuffer[34] = int(uintMatrixMaximum[0][0]);
    outputBuffer[35] = int(uintMatrixMaximum[0][1]);
    outputBuffer[36] = int(uintMatrixMaximum[1][0]);
    outputBuffer[37] = int(uintMatrixMaximum[1][1]);

    float2x2 floatMatrixMinimum = genericMinimum(
        float2x2(2.5, -1.0, 7.0, 4.0), float2x2(4.0, 3.0, 2.0, 4.0));
    float2x2 floatMatrixMaximum = genericMaximum(
        float2x2(2.5, -1.0, 7.0, 4.0), float2x2(4.0, 3.0, 2.0, 4.0));
    outputBuffer[38] = int(floatMatrixMinimum[0][0] * 10.0);
    outputBuffer[39] = int(floatMatrixMinimum[0][1] * 10.0);
    outputBuffer[40] = int(floatMatrixMinimum[1][0] * 10.0);
    outputBuffer[41] = int(floatMatrixMinimum[1][1] * 10.0);
    outputBuffer[42] = int(floatMatrixMaximum[0][0] * 10.0);
    outputBuffer[43] = int(floatMatrixMaximum[0][1] * 10.0);
    outputBuffer[44] = int(floatMatrixMaximum[1][0] * 10.0);
    outputBuffer[45] = int(floatMatrixMaximum[1][1] * 10.0);
}

//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -dx12 -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-cuda -compute -shaderobj
// Integer matrices are not supported in WGSL.
//DISABLE_TEST(compute):COMPARE_COMPUTE_EX:-wgpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-mtl -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-llvm -compute -shaderobj
//TEST:SIMPLE(filecheck=CONCRETE): -target spirv-asm -entry concreteMain -stage compute
//TEST:SIMPLE(filecheck=GENERIC): -target spirv-asm -entry genericCodegenMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry genericMatrixCodegenMain -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry genericVectorCodegenMain -stage compute

// Verify that min and max accept scalar, vector, and matrix operands when generic code knows only
// that the element type belongs to the sealed builtin arithmetic domain.

// Also verify that concrete calls and calls dispatched through the broader arithmetic overload
// both emit target-native shaped instructions rather than scalarizing.

// CONCRETE-DAG: %[[V3_INT:[A-Za-z0-9_]+]] = OpTypeVector %int 3
// CONCRETE-DAG: %[[V2_UINT:[A-Za-z0-9_]+]] = OpTypeVector %uint 2
// CONCRETE-DAG: %[[V4_FLOAT:[A-Za-z0-9_]+]] = OpTypeVector %float 4
// CONCRETE: OpExtInst %[[V3_INT]] {{.*}} SMin
// CONCRETE: OpExtInst %[[V2_UINT]] {{.*}} UMax
// CONCRETE: OpExtInst %[[V4_FLOAT]] {{.*}} FMax

// GENERIC-DAG: %[[V3_INT:[A-Za-z0-9_]+]] = OpTypeVector %int 3
// GENERIC-DAG: %[[V2_INT:[A-Za-z0-9_]+]] = OpTypeVector %int 2
// GENERIC-DAG: %[[V2_UINT:[A-Za-z0-9_]+]] = OpTypeVector %uint 2
// GENERIC-DAG: %[[V2_FLOAT:[A-Za-z0-9_]+]] = OpTypeVector %float 2
// GENERIC-DAG: %[[V4_FLOAT:[A-Za-z0-9_]+]] = OpTypeVector %float 4
// GENERIC: OpExtInst %[[V3_INT]] {{.*}} SMin
// GENERIC: OpExtInst %[[V3_INT]] {{.*}} SMax
// GENERIC: OpExtInst %[[V2_UINT]] {{.*}} UMin
// GENERIC: OpExtInst %[[V2_UINT]] {{.*}} UMax
// GENERIC: OpExtInst %[[V4_FLOAT]] {{.*}} FMin
// GENERIC: OpExtInst %[[V4_FLOAT]] {{.*}} FMax
// GENERIC: OpExtInst %[[V2_INT]] {{.*}} SMin
// GENERIC: OpExtInst %[[V2_INT]] {{.*}} SMax
// GENERIC: OpExtInst %[[V2_UINT]] {{.*}} UMin
// GENERIC: OpExtInst %[[V2_UINT]] {{.*}} UMax
// GENERIC: OpExtInst %[[V2_FLOAT]] {{.*}} FMin
// GENERIC: OpExtInst %[[V2_FLOAT]] {{.*}} FMax

// HLSL-LABEL: row_major int2x2 genericMaximum_{{[0-9]+}}(row_major int2x2
// HLSL: return max(left_{{[0-9]+}}, right_{{[0-9]+}});
// HLSL-LABEL: row_major int2x2 genericMinimum_{{[0-9]+}}(row_major int2x2
// HLSL: return min(left_{{[0-9]+}}, right_{{[0-9]+}});

// WGSL-LABEL: fn genericMaximum_{{[0-9]+}}({{.*}}vec4<f32>{{.*}}) -> vec4<f32>
// WGSL: return max(left_{{[0-9]+}}, right_{{[0-9]+}});
// WGSL-LABEL: fn genericMinimum_{{[0-9]+}}({{.*}}vec4<f32>{{.*}}) -> vec4<f32>
// WGSL: return min(left_{{[0-9]+}}, right_{{[0-9]+}});

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;
RWStructuredBuffer<int3> concreteIntOutput;
RWStructuredBuffer<uint2> concreteUIntOutput;
RWStructuredBuffer<float4> concreteFloatOutput;
RWStructuredBuffer<int2x2> genericIntMatrixOutput;
RWStructuredBuffer<uint2x2> genericUIntMatrixOutput;
RWStructuredBuffer<float2x2> genericFloatMatrixOutput;

[numthreads(1, 1, 1)]
void concreteMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int3 signedValue = int3(dispatchThreadID);
    uint2 unsignedValue = dispatchThreadID.xy;
    float4 floatingValue = float4(float3(dispatchThreadID), 1.0);

    concreteIntOutput[0] = min(signedValue, int3(4, 5, 6));
    concreteUIntOutput[0] = max(unsignedValue, uint2(7, 8));
    concreteFloatOutput[0] = max(floatingValue, float4(1.0, 2.0, 3.0, 4.0));
}

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

matrix<T, R, C, L> genericMinimum<
    T : __BuiltinArithmeticType,
    let R : int,
    let C : int,
    let L : MatrixLayoutMode>(
    matrix<T, R, C, L> left,
    matrix<T, R, C, L> right)
{
    return min(left, right);
}

matrix<T, R, C, L> genericMaximum<
    T : __BuiltinArithmeticType,
    let R : int,
    let C : int,
    let L : MatrixLayoutMode>(
    matrix<T, R, C, L> left,
    matrix<T, R, C, L> right)
{
    return max(left, right);
}

[numthreads(1, 1, 1)]
void genericCodegenMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int3 signedValue = int3(dispatchThreadID);
    uint2 unsignedValue = dispatchThreadID.xy;
    float4 floatingValue = float4(float3(dispatchThreadID), 1.0);

    concreteIntOutput[0] = genericMinimum(signedValue, int3(4, 5, 6));
    concreteIntOutput[1] = genericMaximum(signedValue, int3(4, 5, 6));
    concreteUIntOutput[0] = genericMinimum(unsignedValue, uint2(7, 8));
    concreteUIntOutput[1] = genericMaximum(unsignedValue, uint2(7, 8));
    concreteFloatOutput[0] = genericMinimum(floatingValue, float4(1.0, 2.0, 3.0, 4.0));
    concreteFloatOutput[1] = genericMaximum(floatingValue, float4(1.0, 2.0, 3.0, 4.0));

    matrix<int, 2, 2, MatrixLayoutMode.RowMajor> matrixValue = int2x2(1, 2, 3, 4);
    matrix<int, 2, 2, MatrixLayoutMode.RowMajor> otherMatrixValue = int2x2(4, 3, 2, 1);
    genericIntMatrixOutput[0] = genericMinimum(matrixValue, otherMatrixValue);
    genericIntMatrixOutput[1] = genericMaximum(matrixValue, otherMatrixValue);

    matrix<uint, 2, 2, MatrixLayoutMode.RowMajor> uintMatrixValue = uint2x2(1, 2, 3, 4);
    matrix<uint, 2, 2, MatrixLayoutMode.RowMajor> otherUIntMatrixValue = uint2x2(4, 3, 2, 1);
    genericUIntMatrixOutput[0] = genericMinimum(uintMatrixValue, otherUIntMatrixValue);
    genericUIntMatrixOutput[1] = genericMaximum(uintMatrixValue, otherUIntMatrixValue);

    matrix<float, 2, 2, MatrixLayoutMode.RowMajor> floatMatrixValue = float2x2(1, 2, 3, 4);
    matrix<float, 2, 2, MatrixLayoutMode.RowMajor> otherFloatMatrixValue =
        float2x2(4, 3, 2, 1);
    genericFloatMatrixOutput[0] = genericMinimum(floatMatrixValue, otherFloatMatrixValue);
    genericFloatMatrixOutput[1] = genericMaximum(floatMatrixValue, otherFloatMatrixValue);
}

[numthreads(1, 1, 1)]
void genericMatrixCodegenMain()
{
    matrix<int, 2, 2, MatrixLayoutMode.RowMajor> left = int2x2(1, 2, 3, 4);
    matrix<int, 2, 2, MatrixLayoutMode.RowMajor> right = int2x2(4, 3, 2, 1);
    genericIntMatrixOutput[0] = genericMinimum(left, right);
    genericIntMatrixOutput[1] = genericMaximum(left, right);
}

[numthreads(1, 1, 1)]
void genericVectorCodegenMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int3 signedValue = int3(dispatchThreadID);
    uint2 unsignedValue = dispatchThreadID.xy;
    float4 floatingValue = float4(float3(dispatchThreadID), 1.0);

    concreteIntOutput[0] = genericMinimum(signedValue, int3(4, 5, 6));
    concreteIntOutput[1] = genericMaximum(signedValue, int3(4, 5, 6));
    concreteUIntOutput[0] = genericMinimum(unsignedValue, uint2(7, 8));
    concreteUIntOutput[1] = genericMaximum(unsignedValue, uint2(7, 8));
    concreteFloatOutput[0] = genericMinimum(floatingValue, float4(1.0, 2.0, 3.0, 4.0));
    concreteFloatOutput[1] = genericMaximum(floatingValue, float4(1.0, 2.0, 3.0, 4.0));
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

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type

// Reverse differentiation creates immutable aggregate updates. Both the original snapshot and
// updated aggregate contribute, and runtime indices prevent constant element selection.
[Differentiable]
float vectorUpdate(float x, no_diff uint index)
{
    float3 value = float3(x, 2 * x, 3 * x);
    float3 before = value;
    value[index] = 4 * x;
    return dot(before, float3(1)) + dot(value, float3(1));
}

[Differentiable]
float arrayUpdate(float x, no_diff uint index)
{
    float2 value[2] = {float2(x, 2 * x), float2(3 * x, 4 * x)};
    float2 before[2] = value;
    value[index][0] = 7 * x;
    return dot(before[0] + before[1] + value[0] + value[1], float2(1));
}

[Differentiable]
float matrixUpdate(float x, no_diff uint index)
{
    float2x3 value = float2x3(x, 2 * x, 3 * x, 4 * x, 5 * x, 6 * x);
    float2x3 before = value;
    value[index][index] = 10 * x;
    return dot(before[0] + before[1] + value[0] + value[1], float3(1));
}

struct Record : IDifferentiable
{
    float scalar;
    float2 vector;
}

[Differentiable]
float recordUpdate(float x, no_diff uint index)
{
    Record value = {x, float2(2 * x, 3 * x)};
    Record before = value;
    value.vector[index] = 8 * x;
    return before.scalar + value.scalar + dot(before.vector + value.vector, float2(1));
}

//TEST_INPUT:ubuffer(data=[1], stride=4):name=indices
StructuredBuffer<uint> indices;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<float> outputs;
// CHECK: 14.0
// CHECK-NEXT: 24.0
// CHECK-NEXT: 47.0
// CHECK-NEXT: 17.0
[numthreads(1, 1, 1)]
void computeMain()
{
    uint index = indices[0];
    var vectorInput = diffPair(2.0f, 0.0f);
    var arrayInput = diffPair(2.0f, 0.0f);
    var matrixInput = diffPair(2.0f, 0.0f);
    var recordInput = diffPair(2.0f, 0.0f);
    bwd_diff(vectorUpdate)(vectorInput, index, 1.0f);
    bwd_diff(arrayUpdate)(arrayInput, index, 1.0f);
    bwd_diff(matrixUpdate)(matrixInput, index, 1.0f);
    bwd_diff(recordUpdate)(recordInput, index, 1.0f);
    outputs[0] = vectorInput.d;
    outputs[1] = arrayInput.d;
    outputs[2] = matrixInput.d;
    outputs[3] = recordInput.d;
}

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -matrix-layout-column-major -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -matrix-layout-column-major -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -matrix-layout-column-major -Xslang -emit-cuda-via-nvvm -Xslang -O3

// CUDA packs each float3 column into 12 bytes: (1, 0, 10), then (0, 1, 20).
// The graphics fixture in compute/non-square-column-major.slang uses a 16-byte column stride.
// These raw six-word CUDA inputs have their own packing contract; the layouts are not shared.
//TEST_INPUT:cbuffer(data=[1.0 0.0 10.0 0.0 1.0 20.0]):name=matrixBuffer
ConstantBuffer<float3x2> matrixBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<float> outputs;

// CHECK: 1.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 0.0
// CHECK-NEXT: 1.0
// CHECK-NEXT: 10.0
// CHECK-NEXT: 20.0
// CHECK-NEXT: 11.0
// CHECK-NEXT: 22.0

[numthreads(1, 1, 1)]
void computeMain()
{
    float3x2 matrix = matrixBuffer;
    // Check every logical element in row order before testing multiplication.
    outputs[0] = matrix[0][0];
    outputs[1] = matrix[0][1];
    outputs[2] = matrix[1][0];
    outputs[3] = matrix[1][1];
    outputs[4] = matrix[2][0];
    outputs[5] = matrix[2][1];
    float2 result = mul(float3(1, 2, 1), matrix);
    outputs[6] = result.x;
    outputs[7] = result.y;
}

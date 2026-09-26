//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST_INPUT:cbuffer(data=[1.0 2.0 3.0 9.0 4.0 5.0 6.0 11.0 7.0 8.0 9.0 10.0 11.0 12.0]):name=settings
//TEST_INPUT:ubuffer(data=[1], stride=4):name=inputs
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputs
// CHECK: 321
// CHECK-NEXT: 9
// CHECK-NEXT: 654
// CHECK-NEXT: 11
// CHECK-NEXT: 987
// CHECK-NEXT: 1320

struct Payload
{
    float3 value;
    float sentinel;
};
struct Settings
{
    Payload first;
    Payload nested;
    float3 values[2];
};
ConstantBuffer<Settings> settings;
StructuredBuffer<uint> inputs;
RWStructuredBuffer<uint> outputs;
uint encode(float3 value)
{
    return uint(value.x + 10 * value.y + 100 * value.z);
}
[numthreads(1, 1, 1)]
void computeMain()
{
    // Compact device float3 occupies twelve bytes; native borrowed float3 uses an LLVM vector.
    // Both layouts must preserve the scalar immediately after each vector and the array stride.
    outputs[0] = encode(settings.first.value);
    outputs[1] = uint(settings.first.sentinel);
    outputs[2] = encode(settings.nested.value);
    outputs[3] = uint(settings.nested.sentinel);
    outputs[4] = encode(settings.values[0]);
    outputs[5] = encode(settings.values[inputs[0]]);
}

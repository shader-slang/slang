//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DGROUP_LAYOUT=DefaultDataLayout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DGROUP_LAYOUT=DefaultDataLayout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DGROUP_LAYOUT=ScalarDataLayout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DGROUP_LAYOUT=ScalarDataLayout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -DGROUP_LAYOUT=CDataLayout
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -DGROUP_LAYOUT=CDataLayout
// CUDA uniform groups keep the CUDA host layout even with an explicit layout operand.
// Bool lanes use bytes (including a noncanonical true); wider vectors use scalar alignment.
// The Half3 padding contains a NaN; neither it nor a neighboring scalar belongs to the value.
//TEST_INPUT:cbuffer(data=[1 2 3 91 4 5 6 92 1073757184 2113946112 93 1157645312 1191200256 94 513 95 131073 2147418115 96 0 1 0 2 0 3 0 97 0 0 1072693248 0 1073741824 0 1074266112 98 0]):name=settings
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputs
// CHECK: 321
// CHECK-NEXT: 91
// CHECK-NEXT: 654
// CHECK-NEXT: 92
// CHECK-NEXT: 321
// CHECK-NEXT: 93
// CHECK-NEXT: 7654
// CHECK-NEXT: 94
// CHECK-NEXT: 11
// CHECK-NEXT: 95
// CHECK-NEXT: 321
// CHECK-NEXT: 96
// CHECK-NEXT: 321
// CHECK-NEXT: 97
// CHECK-NEXT: 321
// CHECK-NEXT: 98

struct Settings
{
    int3 integers;
    uint integerTail;
    uint3 unsignedIntegers;
    uint unsignedTail;
    half3 halfThree;
    uint halfThreeTail;
    half4 halfFour;
    uint halfFourTail;
    bool3 booleans;
    uint booleanTail;
    vector<int16_t, 3> narrowIntegers;
    uint narrowTail;
    vector<uint64_t, 3> wideIntegers;
    uint wideTail;
    double3 doubles;
    uint doubleTail;
};
#ifndef GROUP_LAYOUT
#define GROUP_LAYOUT DefaultDataLayout
#endif
ConstantBuffer<Settings, GROUP_LAYOUT> settings;
RWStructuredBuffer<uint> outputs;

[noinline]
uint encodeThree(float3 value)
{
    return uint(value.x + 10 * value.y + 100 * value.z);
}
[noinline]
uint encodeFour(half4 value)
{
    float4 lanes = float4(value);
    return uint(lanes.x + 10 * lanes.y + 100 * lanes.z + 1000 * lanes.w);
}
[numthreads(1, 1, 1)]
void computeMain()
{
    outputs[0] = encodeThree(float3(settings.integers));
    outputs[1] = settings.integerTail;
    outputs[2] = encodeThree(float3(settings.unsignedIntegers));
    outputs[3] = settings.unsignedTail;
    outputs[4] = encodeThree(float3(settings.halfThree));
    outputs[5] = settings.halfThreeTail;
    outputs[6] = encodeFour(settings.halfFour);
    outputs[7] = settings.halfFourTail;
    outputs[8] = encodeThree(float3(settings.booleans));
    outputs[9] = settings.booleanTail;
    outputs[10] = encodeThree(float3(settings.narrowIntegers));
    outputs[11] = settings.narrowTail;
    outputs[12] = encodeThree(float3(settings.wideIntegers));
    outputs[13] = settings.wideTail;
    outputs[14] = encodeThree(float3(settings.doubles));
    outputs[15] = settings.doubleTail;
}

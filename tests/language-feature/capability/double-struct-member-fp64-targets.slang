// On targets with the `fp64` capability, a struct with a `double` field compiles and runs. A
// field's requirements apply where the struct is used, not to its member functions, so a method may
// call capability-gated operations such as `min` and `copysign`.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute

struct Scaled
{
    double value;
    float bias;

    double limited(double limit) { return min(value, limit) + copysign(1.0LF, value); }
};

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// HLSL: double value_0;
// HLSL: double Scaled_limited_0(
// SPIRV: OpCapability Float64
// SPIRV: OpTypeFloat 64
[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Scaled s;
    s.value = double(tid.x) * 1.5LF - 2.0LF;
    s.bias = 0.25;
    outputBuffer[tid.x] = float(s.limited(2.0LF)) + s.bias;
}

// CHECK: -2.75
// CHECK-NEXT: -1.25
// CHECK-NEXT: 2.25
// CHECK-NEXT: 3.25

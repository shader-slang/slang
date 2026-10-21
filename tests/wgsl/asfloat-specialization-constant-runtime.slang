//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute,vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type

// Runtime companion to asfloat-specialization-constant.slang. `kBits | 1u` is hoisted to module
// scope with the spec-const rate, and `d` is read twice, so it is bound to a name; that name must
// be a WGSL `var`, because its initializer is an override-expression.

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;
[vk::constant_id(0)] const uint kBits = 0x3F800000u;
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float d = asfloat(kBits | 1u);
    outputBuffer[tid.x] = asuint(d);
    outputBuffer[tid.x + 1] = asuint(d * 2.0);
}
// 0x3F800001, 0x40000001
// BUF: {{^}}1065353217{{$}}
// BUF-NEXT: {{^}}1073741825{{$}}

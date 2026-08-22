// Runtime coverage for `__getAddress` on a RasterizerOrderedStructuredBuffer element
// (shader-slang/slang#12581). ROV is a rasterizer construct that nvrtc/CPU do not accept
// ("RasterizerOrderedStructuredBuffer is not a template"), so this is Vulkan-only, where it
// lowers to a storage buffer. SPIR-V compile acceptance of the full accepted set is pinned in
// get-address-buffer-element-accepted.slang; this proves the address is executed correctly.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

//TEST_INPUT:ubuffer(data=[100 200 300], stride=4):name=rovBuffer
RasterizerOrderedStructuredBuffer<int> rovBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int* pr = __getAddress(rovBuffer[1]);
    // CHECK: 200
    outputBuffer[0] = *pr;

    *pr = 55;
    // CHECK: 55
    outputBuffer[1] = rovBuffer[1];
}

// Regression test for GitHub issue #8455:
// ParameterBlock<DescriptorHandle<Buffer<T>>> caused an infinite loop
// in _unwrapParameterGroups when creating shader object layouts,
// because Buffer<T> is a TextureType (not a StructuredBuffer) whose
// resource shape caused a `break` that only exited the switch, not
// the enclosing for(;;) loop.
//
// This test exercises the shader object layout creation path on Metal.
// The param variable is declared but not read, so the test validates
// that layout creation completes without hanging.

//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type

ParameterBlock<DescriptorHandle<Buffer<uint>>> param;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    outputBuffer[dtid.x] = dtid.x;
}

// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3

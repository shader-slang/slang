//TEST:SIMPLE(filecheck=METAL):-stage compute -entry computeMain -target metal
//TEST:SIMPLE(filecheck=SPIRV):-stage compute -entry computeMain -target spirv

// Regression test for shader-slang/slang#8455.
//
// Compiling code that uses `ParameterBlock<DescriptorHandle<Buffer<T>>>` and
// then exercising the gfx runtime reflection helper `_unwrapParameterGroups`
// used to spin in an infinite loop because the helper's switch had a `break`
// that fell back into the enclosing `for(;;)` without making progress for
// non-StructuredBuffer Resource shapes.  The fix returns the current layout
// for those shapes; this test pins the language-side surface so a regression
// here would also be caught at compile time.

// METAL: computeMain
// SPIRV: OpEntryPoint

ParameterBlock<DescriptorHandle<Buffer<uint>>> param;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    outputBuffer[dtid.x] = param[0];
}

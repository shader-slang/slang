//TEST:SIMPLE(filecheck=METAL):-stage compute -entry computeMain -target metal

// Compile-only documentation test for shader-slang/slang#8455.
//
// The runtime hang is in `_unwrapParameterGroups` in
// `tools/gfx/renderer-shared.h` (legacy gfx layer): the Resource arm used
// `break`, which only exited the switch and re-entered the enclosing
// `for(;;)` with `typeLayout` unchanged whenever the resource shape was
// not `SLANG_STRUCTURED_BUFFER`. `ParameterBlock<DescriptorHandle<Buffer<T>>>`
// on bindless targets reaches that arm and spins forever.
//
// Note: slang-test exercises render paths via slang-rhi, whose copy of the
// helper (`external/slang-rhi/src/shader-object.h`) is already correct, so
// the repo's render-test cannot reproduce the hang. The legacy
// `tools/gfx/` library is consumed by downstream users and is the actual
// site of the bug; this test pins the language-side surface so a regression
// in compiling the pattern would still be caught here.

// METAL: computeMain
ParameterBlock<DescriptorHandle<Buffer<uint>>> param;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 dtid : SV_DispatchThreadID)
{
    outputBuffer[dtid.x] = param[0];
}

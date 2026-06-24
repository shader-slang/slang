//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Negative gate (issue #11538).
//
// Without `[Shader64BitIndexing]` (and without the `spvShader64BitIndexingEXT` capability anywhere
// in the call graph), none of the capability, extension, or execution mode may appear. Guards that
// Half B is opt-in and does not emit the extension unconditionally.

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = dispatchThreadID.x;
}

// CHECK-NOT: Shader64BitIndexingEXT
// CHECK-NOT: SPV_EXT_shader_64bit_indexing

//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Entry-point-attributed (issue #11538, Approach Y).
//
// `[Shader64BitIndexing]` on the entry point itself injects the `spvShader64BitIndexingEXT`
// capability requirement (Half A) and lifts an `IRShader64BitIndexingDecoration` onto the entry
// point (Half B), which the SPIR-V back-end lowers to the capability, extension, and execution
// mode below. No 64-bit index needs to actually appear in the body: the attribute is the marker.

RWStructuredBuffer<int> outputBuffer;

[Shader64BitIndexing]
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = dispatchThreadID.x;
}

// CHECK-DAG: OpCapability Shader64BitIndexingEXT
// CHECK-DAG: OpExtension "SPV_EXT_shader_64bit_indexing"
// CHECK-DAG: OpExecutionMode %computeMain Shader64BitIndexingEXT

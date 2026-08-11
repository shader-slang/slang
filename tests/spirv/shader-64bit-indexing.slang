//TEST:SIMPLE(filecheck=CHECK): -target spirv

// `[Shader64BitIndexing]` applied directly to the entry point (issue #11538).
//
// The attribute on `computeMain` unions the `spvShader64BitIndexingEXT` capability into the entry
// point's inferred requirement set; entry-point lowering then attaches a decoration that the
// SPIR-V back-end emits as the capability, extension, and execution mode checked below. No 64-bit
// index needs to appear in the body — the attribute alone is the opt-in marker.

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

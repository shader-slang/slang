//TEST:SIMPLE(filecheck=CHECK): -target spirv

// `[Shader64BitIndexing]` on a callee, not on the entry point (issue #11538).
//
// The attribute sits on `helper`. The `spvShader64BitIndexingEXT` requirement propagates up the
// call graph to `computeMain`, and entry-point lowering attaches the decoration to `computeMain`
// (attaching it to the callee would be inert, since SPIR-V execution modes are entry-point scoped).
// The capability, extension, and execution mode must therefore land on the entry point.

RWStructuredBuffer<int> outputBuffer;

[Shader64BitIndexing]
int helper(int n)
{
    return n + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    outputBuffer[i] = helper(outputBuffer[i]);
}

// CHECK-DAG: OpCapability Shader64BitIndexingEXT
// CHECK-DAG: OpExtension "SPV_EXT_shader_64bit_indexing"
// CHECK-DAG: OpExecutionMode %computeMain Shader64BitIndexingEXT

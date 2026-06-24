//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Callee-attributed call-graph lift (issue #11538, Approach Y, mechanism B-i).
//
// `[Shader64BitIndexing]` sits on a helper, NOT the entry point. Half A propagates the
// `spvShader64BitIndexingEXT` requirement up the call graph to `computeMain`; the entry-point
// lowering loop then lifts the decoration onto `computeMain` (a decoration left on the callee
// would be inert, since SPIR-V execution modes are entry-point scoped). The capability, extension,
// and execution mode must therefore land on the entry point.

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

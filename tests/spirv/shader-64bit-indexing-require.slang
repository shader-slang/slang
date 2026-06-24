//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -capability spvShader64BitIndexingEXT

// Capability-atom-driven emission (issue #11538).
//
// Half B keys off the `spvShader64BitIndexingEXT` capability atom in the entry point's inferred
// requirement set, not off the `[Shader64BitIndexing]` attribute specifically. So requesting the
// atom directly with `[require(spvShader64BitIndexingEXT)]` on a callee drives the same
// capability/extension/execution-mode emission on the entry point. This also guards the atom's
// existence in slang-capabilities.capdef: removing it makes `[require(...)]` an unknown-capability
// error and this file fails to compile. `-capability` keeps the `[require]` from tripping the
// implicit-capability-upgrade warning under restrictive capability checking.

RWStructuredBuffer<int> outputBuffer;

[require(spvShader64BitIndexingEXT)]
int requireShader64BitIndexing(int n)
{
    return n * 2;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    outputBuffer[i] = requireShader64BitIndexing(outputBuffer[i]);
}

// CHECK-DAG: OpCapability Shader64BitIndexingEXT
// CHECK-DAG: OpExtension "SPV_EXT_shader_64bit_indexing"
// CHECK-DAG: OpExecutionMode %computeMain Shader64BitIndexingEXT

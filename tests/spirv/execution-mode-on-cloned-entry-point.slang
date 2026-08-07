//TEST:SIMPLE(filecheck=CHECK): -target spirv -fvk-use-entrypoint-name -O0 -emit-spirv-directly
//TEST:SIMPLE(filecheck=PRESENT): -target spirv -fvk-use-entrypoint-name -O0 -emit-spirv-directly

// An entry point that is *also called* by another entry point gets cloned by
// `fixEntryPointCallsites`: the callsite is redirected to a copy that acts as an ordinary function.
// That copy loses `IREntryPointDecoration`, so it must also lose every decoration that is only
// meaningful on an entry point -- otherwise the SPIR-V emitter has an `OpExecutionMode` to emit for a
// plain `OpFunction`, which the spec does not allow.
//
// `[Shader64BitIndexing]` reaches `computeMainB` by propagating from `helper` up the call graph, so
// this covers the decorations that arrive at an entry point indirectly rather than as a written
// attribute.

// No execution mode may name the clone. `-O0` keeps the call from being inlined, which is what makes
// the clone exist at all.
// CHECK-NOT: OpExecutionMode %computeMainB_0

// PRESENT-DAG: OpEntryPoint GLCompute %computeMainB
// PRESENT-DAG: OpExecutionMode %computeMainB Shader64BitIndexingEXT
// PRESENT-DAG: OpEntryPoint GLCompute %computeMainA

RWStructuredBuffer<int> outputBuffer;

[Shader64BitIndexing]
int helper(int n)
{
    return n + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainB()
{
    outputBuffer[0] = helper(1);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainA()
{
    computeMainB();
}

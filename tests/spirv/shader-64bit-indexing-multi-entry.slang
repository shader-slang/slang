//TEST:SIMPLE(filecheck=CHECK): -target spirv -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=CHECK_NEG): -target spirv -fvk-use-entrypoint-name

// Per-entry-point scoping of the execution mode (issue #11538).
//
// Two compute entry points share one module. `computeMainA` calls `helper`, which carries
// `[Shader64BitIndexing]`, so the requirement propagates to `computeMainA` and the
// `Shader64BitIndexingEXT` execution mode is lifted onto it. `computeMainB` does not call `helper`
// and carries no such requirement, so it must NOT receive the execution mode. The capability and
// extension are module-global (one entry point needs them), but the execution mode is per entry
// point — that per-entry-point scoping is exactly what the lift enforces. The negative is checked
// in its own FileCheck pass so a lone CHECK-NOT scans the whole module without ordering hazards.

RWStructuredBuffer<int> outputBuffer;

[Shader64BitIndexing]
int helper(int n)
{
    return n + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainA(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int i = dispatchThreadID.x;
    outputBuffer[i] = helper(outputBuffer[i]);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMainB(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = dispatchThreadID.x;
}

// CHECK-DAG: OpCapability Shader64BitIndexingEXT
// CHECK-DAG: OpExtension "SPV_EXT_shader_64bit_indexing"
// CHECK-DAG: OpExecutionMode %computeMainA Shader64BitIndexingEXT

// CHECK_NEG-NOT: OpExecutionMode %computeMainB Shader64BitIndexingEXT

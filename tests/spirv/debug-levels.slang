//TEST:SIMPLE(filecheck=CHECK_NONE): -target spirv -g0 -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_MINIMAL): -target spirv -g1 -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_STANDARD): -target spirv -g2 -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_MAXIMAL): -target spirv -g3 -emit-spirv-directly

// Test that verifies different debug information levels generate appropriate debug info:
// - g0/none: No debug info at all
// - g1/minimal: Line numbers only (OpLine) but no variable debug info, no DebugInlinedAt
// - g2/standard: Full debug info including variables (DebugLocalVariable), DebugInlinedAt for inlined functions
// - g3/maximal: same as standard when targeting spirv

// CHECK_NONE-NOT: OpLine
// CHECK_NONE-NOT: OpDebugSource
// CHECK_NONE-NOT: DebugLocalVariable
// CHECK_NONE-NOT: DebugInlinedAt

// CHECK_MINIMAL-DAG: OpString "{{.*}}debug-levels.slang"
// CHECK_MINIMAL-DAG: OpLine
// CHECK_MINIMAL-NOT: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK_MINIMAL-NOT: DebugLocalVariable
// CHECK_MINIMAL-NOT: DebugInlinedAt

// CHECK_STANDARD-DAG: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK_STANDARD-DAG: DebugFunction
// CHECK_STANDARD-DAG: DebugLine
// CHECK_STANDARD-DAG: DebugInlinedAt
// CHECK_STANDARD-DAG: DebugLocalVariable

// CHECK_MAXIMAL-DAG: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK_MAXIMAL-DAG: DebugFunction
// CHECK_MAXIMAL-DAG: DebugLine
// CHECK_MAXIMAL-DAG: DebugInlinedAt
// CHECK_MAXIMAL-DAG: DebugLocalVariable

struct MyStruct
{
    float4 value;
    int index;
};

RWStructuredBuffer<MyStruct> outputBuffer;

// Helper function that will be inlined - should generate DebugInlinedAt at g2/g3
float computeValue(float x, float y)
{
    float temp = x * 2.0f + y * 0.5f;
    return temp * temp;
}

// Another inline helper to test nested inlining
[ForceInline]
int processIndex(uint y, uint z)
{
    int result = int(y) * 10;
    result += int(z);
    return result;
}

[numthreads(8, 8, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Local variable that should appear in debug info at standard level and above
    // Call inline helper functions - should generate DebugInlinedAt at g2/g3
    float localValue = computeValue(float(tid.x), float(tid.y));
    int localIndex = processIndex(tid.y, tid.z);
    
    MyStruct result;
    result.value = float4(localValue, tid.y, tid.z, 1.0);
    result.index = localIndex;
    
    outputBuffer[tid.x + tid.y * 8] = result;
}


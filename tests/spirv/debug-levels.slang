//TEST:SIMPLE(filecheck=CHECK_NONE): -target spirv -g0 -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_MINIMAL): -target spirv -g1 -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_STANDARD): -target spirv -g2 -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_MAXIMAL): -target spirv -g3 -emit-spirv-directly

// Test that verifies different debug information levels generate appropriate debug info:
// - g0/none: No debug info at all
// - g1/minimal: Line numbers only (OpLine) but no variable debug info
// - g2/standard: Full debug info including variables (OpDebugLocalVariable), with source content embedded
// - g3/maximal: same as standard when targeting spirv

// CHECK_NONE-NOT: OpLine
// CHECK_NONE-NOT: OpDebugSource
// CHECK_NONE-NOT: OpDebugLocalVariable

// CHECK_MINIMAL: OpString "{{.*}}debug-levels.slang"
// CHECK_MINIMAL: OpLine
// CHECK_MINIMAL-NOT: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK_MINIMAL-NOT: OpDebugLocalVariable

// CHECK_STANDARD: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK_STANDARD: DebugFunction
// CHECK_STANDARD: DebugLine

// CHECK_MAXIMAL: OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
// CHECK_MAXIMAL: DebugFunction
// CHECK_MAXIMAL: DebugLine

struct MyStruct
{
    float4 value;
    int index;
};

RWStructuredBuffer<MyStruct> outputBuffer;

[numthreads(8, 8, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Local variable that should appear in debug info at standard level and above
    float localValue = tid.x * 2.0f;
    int localIndex = int(tid.y);
    
    MyStruct result;
    result.value = float4(localValue, tid.y, tid.z, 1.0);
    result.index = localIndex;
    
    outputBuffer[tid.x + tid.y * 8] = result;
}


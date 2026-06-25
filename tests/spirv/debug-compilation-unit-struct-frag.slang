//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage fragment -g2 -emit-spirv-directly

// Regression test for https://github.com/shader-slang/slang/issues/10906
// A fragment shader with struct-typed input parameters previously caused a crash in
// findDebugScope when emitting DebugGlobalVariable for the promoted entry-point parameters.

struct FragInput
{
    float4 pos : SV_Position;
    float4 color : COLOR;
}

[shader("fragment")]
float4 main(FragInput input) : SV_Target
{
    return input.color;
}

// CHECK: DebugCompilationUnit
// CHECK: DebugGlobalVariable

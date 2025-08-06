//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly

struct VertexInput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

float4 main(VertexInput input) : SV_TARGET
{
    return input.position * input.color;
}

// The key test: verify that individual struct member debug variables are created
// These should appear in OpName instructions early in the output
// CHECK: OpName %input_position "input.position"
// CHECK: OpName %input_color{{(_[0-9]+)?}} "input.color"

// Verify basic debug infrastructure exists
// CHECK: DebugCompilationUnit
// CHECK: DebugFunction

// Verify that DebugLocalVariable is generated for the entry point parameter
// CHECK: %input = OpExtInst %void %{{[0-9]+}} DebugLocalVariable

// Verify that DebugLocalVariable is generated for individual struct members
// CHECK: %input_position = OpExtInst %void %{{[0-9]+}} DebugLocalVariable
// CHECK: %input_color{{(_[0-9]+)?}} = OpExtInst %void %{{[0-9]+}} DebugLocalVariable

// Verify that DebugValue instructions are generated
// CHECK: DebugValue %input
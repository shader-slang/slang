//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -g2 -emit-spirv-directly

// This test is to make sure Slang emits Debug info for the member variables
// of the entry point param after the entry point param is legalized, unused,
// and removed.

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

// Verify that DebugLocalVariable is generated for the entry point parameter
// CHECK: %input = OpExtInst %void %{{[0-9]+}} DebugLocalVariable

// Verify that DebugLocalVariable is generated for individual struct members
// CHECK: %input_position = OpExtInst %void %{{[0-9]+}} DebugLocalVariable
// CHECK: %input_color{{[^ ]*}} = OpExtInst %void %{{[0-9]+}} DebugLocalVariable

// Verify that DebugValue instructions are generated for the entry point param
// CHECK: DebugValue %input

// Verify that DebugValue instructions are generated for the members
// CHECK: DebugDeclare %input_position
// CHECK: DebugDeclare %input_color

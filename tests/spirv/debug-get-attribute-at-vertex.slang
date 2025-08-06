//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric -g2

// This test is to make sure that the debug information for the entry point param
// is emitted properly even when a member variable of the entry point param is
// used on GetAttributeAtVertex().
//
// Note that `GetAttributeAtVertex()` requires the argument to be a direct
// reference to a varying-input; not a local variable copied the value from it.

struct FragmentInput
{
    float4 position : SV_Position;
    pervertex float4 color : COLOR0;
};

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    float3 bary = float3(0.33, 0.33, 0.34);
    float4 interpolated = bary.x * GetAttributeAtVertex(input.color, 0)
                        + bary.y * GetAttributeAtVertex(input.color, 1)
                        + bary.z * GetAttributeAtVertex(input.color, 2);
    return interpolated * input.position;
}

// Verify that debug variables are properly named for struct members
// CHECK: OpName %input_position "input.position"
// CHECK: OpName %input_color{{(_[0-9]+)?}} "input.color"

// Verify the global variable is properly decorated and declared (appears early)
// `Input` means it is a global varying-input.
// CHECK: %input_color_{{[0-9]+}} = OpVariable {{.*}} Input

// Verify debug infrastructure exists
// CHECK: DebugCompilationUnit
// CHECK: DebugFunction

// Verify that DebugLocalVariable is generated for the entry point parameter
// CHECK: %input = OpExtInst %void %{{[0-9]+}} DebugLocalVariable

// Verify that DebugLocalVariable is generated for individual struct members
// This is crucial - even when color is accessed via GetAttributeAtVertex,
// we should still have debug variables for the original parameter members
// CHECK: %input_position = OpExtInst %void %{{[0-9]+}} DebugLocalVariable
// CHECK: %input_color{{(_[0-9]+)?}} = OpExtInst %void %{{[0-9]+}} DebugLocalVariable

// Verify that DebugValue instructions map the debug variables correctly
// CHECK: DebugValue %input

// Verify GetAttributeAtVertex generates proper SPIR-V access patterns
// The key test: ensure it accesses the global variable directly, not a local copy
// CHECK: OpAccessChain {{.*}} %input_color_{{[0-9]+}} %uint_0
// CHECK: OpAccessChain {{.*}} %input_color_{{[0-9]+}} %uint_1
// CHECK: OpAccessChain {{.*}} %input_color_{{[0-9]+}} %uint_2


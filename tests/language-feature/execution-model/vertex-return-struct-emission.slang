// Test that a vertex shader returning a struct (with multiple
// varying outputs + SV_Position) lowers correctly across SPIRV and
// GLSL. Exercises the entry-point-result-type → varying-out
// translation in `slang-ir-legalize-varying-params.cpp`, where
// `entryPointFunc->getResultType()` is decomposed into one or more
// varying parameters.
//
// The PR's existing `cross-stage-vertex-fragment-emission.slang`
// covers the HLSL emission path; this test extends to the SPIRV
// and GLSL paths which take different code through
// _createLegalVaryingVal.

//TEST:SIMPLE(filecheck=CHECK_SPV):  -target spirv-asm -entry vertMain -stage vertex -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl      -entry vertMain -stage vertex

struct VOut
{
    float3 color    : COLOR;
    float2 uv       : TEXCOORD0;
    float4 position : SV_Position;
};

[shader("vertex")]
VOut vertMain(float3 pos : POSITION, float3 inColor : COLOR, float2 inUv : TEXCOORD0)
{
    VOut o;
    o.color    = inColor * 0.5;
    o.uv       = inUv;
    o.position = float4(pos, 1.0);
    return o;
}

// SPIRV: each non-builtin varying gets a Location decoration; the
// builtin (SV_Position) gets a BuiltIn decoration.
// CHECK_SPV-DAG: OpEntryPoint Vertex %vertMain "main" %entryPointParam_vertMain_color %entryPointParam_vertMain_uv %gl_Position %pos %inColor %inUv
// CHECK_SPV-DAG: OpDecorate %pos Location 0
// CHECK_SPV-DAG: OpDecorate %inColor Location 1
// CHECK_SPV-DAG: OpDecorate %inUv Location 2
// CHECK_SPV-DAG: OpDecorate %entryPointParam_vertMain_color Location 0
// CHECK_SPV-DAG: OpDecorate %entryPointParam_vertMain_uv Location 1
// CHECK_SPV-DAG: OpDecorate %gl_Position BuiltIn Position
// CHECK_SPV-DAG: %entryPointParam_vertMain_color = OpVariable %_ptr_Output_v3float Output
// CHECK_SPV-DAG: %entryPointParam_vertMain_uv = OpVariable %_ptr_Output_v2float Output
// CHECK_SPV-DAG: %gl_Position = OpVariable %_ptr_Output_v4float Output
// CHECK_SPV: OpStore %entryPointParam_vertMain_color
// CHECK_SPV: OpStore %entryPointParam_vertMain_uv
// CHECK_SPV: OpStore %gl_Position

// GLSL: each user varying becomes an `out` interface variable with
// a `layout(location = N)` decoration; SV_Position becomes
// `gl_Position`.
// CHECK_GLSL: layout(location = 0)
// CHECK_GLSL-NEXT: out vec3 entryPointParam_vertMain_color_0;
// CHECK_GLSL: layout(location = 1)
// CHECK_GLSL-NEXT: out vec2 entryPointParam_vertMain_uv_0;
// CHECK_GLSL: layout(location = 0)
// CHECK_GLSL-NEXT: in vec3 pos_0;
// CHECK_GLSL: layout(location = 1)
// CHECK_GLSL-NEXT: in vec3 inColor_0;
// CHECK_GLSL: layout(location = 2)
// CHECK_GLSL-NEXT: in vec2 inUv_0;
// CHECK_GLSL: entryPointParam_vertMain_color_0 = o_0.color_0;
// CHECK_GLSL: entryPointParam_vertMain_uv_0 = _S1.uv_0;
// CHECK_GLSL: gl_Position = _S1.position_0;

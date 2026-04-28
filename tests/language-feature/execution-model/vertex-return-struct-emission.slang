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
// CHECK_SPV: OpEntryPoint Vertex
// CHECK_SPV: Location
// CHECK_SPV: BuiltIn Position

// GLSL: each user varying becomes an `out` interface variable with
// a `layout(location = N)` decoration; SV_Position becomes
// `gl_Position`.
// CHECK_GLSL: layout(location =
// CHECK_GLSL: out
// CHECK_GLSL: gl_Position

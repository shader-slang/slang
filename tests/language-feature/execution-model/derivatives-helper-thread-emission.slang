// Helper-thread participation: ddx/ddy require neighboring fragments
// in a 2x2 quad. The hardware spawns "helper threads" — threads whose
// fragments would have been culled but execute anyway to compute
// derivatives.
//
// From the docs:
//   "*helper thread* — a thread that is used to compute derivatives,
//    typically for fragment quads. A helper thread does not produce
//    any other side effects, and it does not participate in
//    wave-tangled functions unless otherwise stated."
//
// This test pins emission of the four documented derivative variants
// (ddx, ddx_fine, ddy, ddy_fine) on each backend.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage fragment -entry fragmentMain -emit-spirv-directly

struct FragInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[shader("fragment")]
float4 fragmentMain(FragInput input) : SV_Target
{
    float dxC = ddx(input.uv.x);
    float dxF = ddx_fine(input.uv.x);
    float dyC = ddy(input.uv.y);
    float dyF = ddy_fine(input.uv.y);
    return float4(dxC, dxF, dyC, dyF);
}

// HLSL keeps the literal intrinsic spellings.
// CHECK_HLSL-DAG: ddx
// CHECK_HLSL-DAG: ddx_fine
// CHECK_HLSL-DAG: ddy
// CHECK_HLSL-DAG: ddy_fine

// GLSL emits the dFdx / dFdy family. Coarse / fine variants are
// distinguished via dFdxCoarse / dFdxFine where the GLSL profile
// supports them; otherwise the implementation may fall back to
// the unqualified form.
// CHECK_GLSL-DAG: dFdx
// CHECK_GLSL-DAG: dFdy

// SPIRV emits OpDPdx / OpDPdy and the Coarse / Fine variants.
// CHECK_SPIRV-DAG: OpDPdx
// CHECK_SPIRV-DAG: OpDPdy

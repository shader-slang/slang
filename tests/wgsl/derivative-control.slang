// The fine and coarse derivatives map to WGSL's dpdxFine/dpdyFine/dpdxCoarse/dpdyCoarse and
// fwidthFine/fwidthCoarse (core WGSL), and clip() to a conditional discard.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm

struct FSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

// SPV: OpEntryPoint
// WGSL-DAG: dpdxFine(
// WGSL-DAG: dpdyFine(
// WGSL-DAG: dpdxCoarse(
// WGSL-DAG: dpdyCoarse(
// WGSL-DAG: fwidthFine(
// WGSL-DAG: fwidthCoarse(
// WGSL-DAG: discard;
[shader("fragment")]
float4 fragMain(FSInput input) : SV_Target
{
    clip(input.uv.x - 0.25);
    float2 fine = ddx_fine(input.uv) + ddy_fine(input.uv);
    float2 coarse = ddx_coarse(input.uv) + ddy_coarse(input.uv);
    float w = fwidth_fine(input.uv.x) + fwidth_coarse(input.uv.y);
    float m = ddx_fine(float2x2(input.uv, input.uv))[1].y;
    return float4(fine + coarse, w, m);
}

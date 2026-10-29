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
// clip(x) discards exactly when x < 0.
// WGSL: fn clip_{{[0-9]+}}( [[X:x_[0-9]+]] : f32)
// WGSL-NEXT: {
// WGSL-NEXT: if(([[X]] < 0.0f))
// WGSL-NEXT: {
// WGSL-NEXT: discard;
// WGSL-NEXT: }
// WGSL: @fragment
// WGSL: clip_{{[0-9]+}}({{.*}} - 0.25f);
// WGSL: dpdxFine(
// WGSL: dpdyFine(
// WGSL: dpdxCoarse(
// WGSL: dpdyCoarse(
// WGSL: fwidthFine(
// WGSL: fwidthCoarse(
// The matrix overload runs per row, through a helper.
// WGSL: ddx_fine_{{[0-9]+}}(mat2x2<f32>(
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

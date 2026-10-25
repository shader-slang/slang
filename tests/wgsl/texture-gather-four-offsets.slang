// Gather with four offsets: WGSL's textureGather takes one offset, so component i of the result is
// the (i0, j0) texel, component w, of a gather with offset i (the OpImageGather ConstOffsets /
// textureGatherOffsets rule).
//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl
//TEST:SIMPLE(filecheck=WDOT): -stage compute -entry computeMain -target wgsl
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -stage compute -entry computeMain -target wgsl-spirv-asm

Texture2D<float4> colorTex;
DepthTexture2D depthTex;
SamplerState samp;
SamplerComparisonState cmpSamp;
RWStructuredBuffer<float4> outputBuffer;

// SPV: OpEntryPoint
// Component w of each single-offset gather (3 per call taken into temporaries, the 4th inline).
// WDOT-COUNT-6: : f32 = {{_S[0-9]+}}.w;
// WDOT-NOT: : f32 = {{_S[0-9]+}}.{{[xyz]}};
// Four single-offset gathers per call (offsets may be hoisted into function-scope consts), then
// component w taken from each.
// WGSL-COUNT-4: textureGather((i32(1)), (colorTex_{{[0-9]+}}), (samp_{{[0-9]+}}),
// WGSL: vec4<f32>({{_S[0-9]+}}, {{_S[0-9]+}}, {{_S[0-9]+}}, {{_S[0-9]+}}.w)
// WGSL-COUNT-4: textureGatherCompare((depthTex_{{[0-9]+}}), (cmpSamp_{{[0-9]+}}),
// WGSL: vec4<f32>({{_S[0-9]+}}, {{_S[0-9]+}}, {{_S[0-9]+}}, {{_S[0-9]+}}.w)
[numthreads(1, 1, 1)]
void computeMain()
{
    float2 uv = float2(0.25, 0.75);
    outputBuffer[0] = colorTex.GatherGreen(samp, uv, int2(0, 0), int2(1, 0), int2(0, 1), int2(1, 1));
    outputBuffer[1] = depthTex.GatherCmp(cmpSamp, uv, 0.5, int2(-1, 0), int2(0, -1), int2(1, 0), int2(0, 1));
}

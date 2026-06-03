// Image pipeline entry point. One compute thread per pixel; each
// thread runs the full denoise → tonemap → gamma sequence. Mode
// parameters select the branch arms inside each stage; the host
// driver dispatches the kernel twice (smoke + exhaustive) and the
// coverage delta is the demo's headline.

import denoise;
import tonemap;
import gamma;

[[vk::binding(0, 0)]] RWStructuredBuffer<float4> inputImage;
[[vk::binding(1, 0)]] RWStructuredBuffer<float4> outputImage;

struct PipelineParams
{
    uint imageWidth;
    uint imageHeight;
    uint tonemapOp;       // TonemapOperator
    uint boundaryMode;    // BoundaryMode
    uint gammaMode;       // GammaMode
    float bilateralSigmaS;
    float bilateralSigmaR;
    float exposure;
}

[[vk::binding(2, 0)]] StructuredBuffer<PipelineParams> paramsBuffer;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    PipelineParams params = paramsBuffer[0];
    uint x = tid.x;
    uint y = tid.y;
    if (x >= params.imageWidth || y >= params.imageHeight)
        return;

    // Stage 1: bilateral denoise
    BoundaryMode boundary = BoundaryMode(params.boundaryMode);
    float3 denoised = bilateralDenoisePixel(
        inputImage,
        int(x),
        int(y),
        int(params.imageWidth),
        int(params.imageHeight),
        params.bilateralSigmaS,
        params.bilateralSigmaR,
        boundary);

    // Stage 2: exposure + tonemap (HDR → LDR)
    float3 hdr = denoised * params.exposure;
    TonemapOperator op = TonemapOperator(params.tonemapOp);
    float3 ldr = applyTonemap(hdr, op);

    // Stage 3: gamma encode for display
    GammaMode gm = GammaMode(params.gammaMode);
    float3 encoded = applyGamma(ldr, gm);

    outputImage[y * params.imageWidth + x] = float4(encoded, 1.0);
}

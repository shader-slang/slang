// Metal codegen for depth / multisample-depth textures (issue #12587).
// Each case uses its own entry point so a single slang-test invocation emits exactly one texture's
// call, keeping every positive/negative check bound to that one receiver — a regression in one
// variant cannot be masked by the other's correct output.

// Case 2 (multisample GetDimensions): get_width()/get_height() take no LOD argument. Applies to
// both depth and color multisample textures.
//TEST:SIMPLE(filecheck=DEPTHDIMS): -stage compute -entry depthDimsMain -target metal
//TEST:SIMPLE(filecheck=COLORDIMS): -stage compute -entry colorDimsMain -target metal

// Case 3 (Gather): Metal depth-texture `gather` takes no component argument, but color
// `texture2d::gather` does. Each depth shape (2D, 2D-array, cube, cube-array) must omit
// `metal::component(...)`; the color control must keep it.
//TEST:SIMPLE(filecheck=DEPTHGATHER): -stage compute -entry depthGatherMain -target metal
//TEST:SIMPLE(filecheck=DEPTHGATHERARR): -stage compute -entry depthGatherArrayMain -target metal
//TEST:SIMPLE(filecheck=DEPTHGATHERCUBE): -stage compute -entry depthGatherCubeMain -target metal
//TEST:SIMPLE(filecheck=DEPTHGATHERCUBEARR): -stage compute -entry depthGatherCubeArrayMain -target metal
//TEST:SIMPLE(filecheck=COLORGATHER): -stage compute -entry colorGatherMain -target metal

// The offset variant of Gather (`__texture_gather_offset`) shares the same depth/color component
// rule; Metal supports it only for 2D textures.
//TEST:SIMPLE(filecheck=DEPTHGATHEROFF): -stage compute -entry depthGatherOffsetMain -target metal
//TEST:SIMPLE(filecheck=DEPTHGATHEROFFARR): -stage compute -entry depthGatherOffsetArrayMain -target metal
//TEST:SIMPLE(filecheck=COLORGATHEROFF): -stage compute -entry colorGatherOffsetMain -target metal

DepthTexture2DMS depthMS;
Texture2DMS<float4> colorMS;
DepthTexture2D depthTex;
DepthTexture2DArray depthArray;
DepthTextureCube depthCube;
DepthTextureCubeArray depthCubeArray;
Texture2D<float4> colorTex;
SamplerState samp;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void depthDimsMain()
{
    uint w, h, s;
    depthMS.GetDimensions(w, h, s);
    // DEPTHDIMS: get_width()
    // DEPTHDIMS: get_height()
    // DEPTHDIMS-NOT: get_width(0)
    // DEPTHDIMS-NOT: get_height(0)
    outBuf[0] = float(w + h + s);
}

[numthreads(1, 1, 1)]
void colorDimsMain()
{
    uint w, h, s;
    colorMS.GetDimensions(w, h, s);
    // COLORDIMS: get_width()
    // COLORDIMS: get_height()
    // COLORDIMS-NOT: get_width(0)
    // COLORDIMS-NOT: get_height(0)
    outBuf[0] = float(w + h + s);
}

[numthreads(1, 1, 1)]
void depthGatherMain()
{
    // DEPTHGATHER: gather(
    // DEPTHGATHER-NOT: metal::component
    outBuf[0] = depthTex.Gather(samp, float2(0.5, 0.5)).x;
}

[numthreads(1, 1, 1)]
void depthGatherArrayMain()
{
    // DEPTHGATHERARR: gather(
    // DEPTHGATHERARR-NOT: metal::component
    outBuf[0] = depthArray.Gather(samp, float3(0.5, 0.5, 0)).x;
}

[numthreads(1, 1, 1)]
void depthGatherCubeMain()
{
    // DEPTHGATHERCUBE: gather(
    // DEPTHGATHERCUBE-NOT: metal::component
    outBuf[0] = depthCube.Gather(samp, float3(0.5, 0.5, 0.5)).x;
}

[numthreads(1, 1, 1)]
void depthGatherCubeArrayMain()
{
    // DEPTHGATHERCUBEARR: gather(
    // DEPTHGATHERCUBEARR-NOT: metal::component
    outBuf[0] = depthCubeArray.Gather(samp, float4(0.5, 0.5, 0.5, 0)).x;
}

[numthreads(1, 1, 1)]
void colorGatherMain()
{
    // COLORGATHER: gather({{.*}}metal::component(
    outBuf[0] = colorTex.Gather(samp, float2(0.5, 0.5)).x;
}

[numthreads(1, 1, 1)]
void depthGatherOffsetMain()
{
    // DEPTHGATHEROFF: gather(
    // DEPTHGATHEROFF-NOT: metal::component
    outBuf[0] = depthTex.Gather(samp, float2(0.5, 0.5), int2(1, 1)).x;
}

[numthreads(1, 1, 1)]
void depthGatherOffsetArrayMain()
{
    // DEPTHGATHEROFFARR: gather(
    // DEPTHGATHEROFFARR-NOT: metal::component
    outBuf[0] = depthArray.Gather(samp, float3(0.5, 0.5, 0), int2(1, 1)).x;
}

[numthreads(1, 1, 1)]
void colorGatherOffsetMain()
{
    // COLORGATHEROFF: gather({{.*}}metal::component(
    outBuf[0] = colorTex.Gather(samp, float2(0.5, 0.5), int2(1, 1)).x;
}

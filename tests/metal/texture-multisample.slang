// Metal `texture2d_ms`/`depth2d_ms` `read` require a `uint2` coordinate, and the array variants
// require a `uint` array index; the signed-integer HLSL location and array index must be converted,
// not passed through. All four multisample variants (color/depth x non-array/array) are checked in
// isolated entry points so each positive/negative check binds to a single `read` receiver.
// (The color case is also tracked by #8457.)

//TEST:SIMPLE(filecheck=COLOR): -stage compute -entry colorMain -target metal -DEMIT_SOURCE
//TEST:SIMPLE(filecheck=DEPTH): -stage compute -entry depthMain -target metal -DEMIT_SOURCE
//TEST:SIMPLE(filecheck=COLORARR): -stage compute -entry colorArrayMain -target metal -DEMIT_SOURCE
//TEST:SIMPLE(filecheck=DEPTHARR): -stage compute -entry depthArrayMain -target metal -DEMIT_SOURCE
//TEST:SIMPLE(filecheck=METALLIB): -stage compute -entry colorMain -target metallib -DEMIT_SOURCE

// METALLIB-NOT: error :
// METALLIB: @colorMain

Texture2DMS<float4> colorMS;
DepthTexture2DMS depthMS;
Texture2DMSArray<float4> colorMSArray;
DepthTexture2DMSArray depthMSArray;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void colorMain()
{
    // COLOR: read(vec<uint,2>(
    // COLOR-NOT: read((
    outBuf[0] = colorMS.Load(int2(0, 0), 0).x;
}

[numthreads(1, 1, 1)]
void depthMain()
{
    // DEPTH: read(vec<uint,2>(
    // DEPTH-NOT: read((
    outBuf[0] = depthMS.Load(int2(0, 0), 0);
}

[numthreads(1, 1, 1)]
void colorArrayMain()
{
    // COLORARR: read(vec<uint,2>({{.*}}), uint({{.*}}), uint(
    // COLORARR-NOT: read((
    outBuf[0] = colorMSArray.Load(int3(0, 0, 0), 0).x;
}

[numthreads(1, 1, 1)]
void depthArrayMain()
{
    // DEPTHARR: read(vec<uint,2>({{.*}}), uint({{.*}}), uint(
    // DEPTHARR-NOT: read((
    outBuf[0] = depthMSArray.Load(int3(0, 0, 0), 0);
}

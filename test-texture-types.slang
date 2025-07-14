typedef uint4 Element;

[shader("compute")]
[numthreads(1,1,1)]
void copyTexture(
    uint3 tid : SV_DispatchThreadID,
    uniform Texture3D<uint> srcTexture,
    uniform [format("r8ui")] RWTexture3D<uint> dstTexture)
{
    uint3 srcDims;
    srcTexture.GetDimensions(srcDims.x, srcDims.y, srcDims.z);
    uint3 dstDims;
    srcTexture.GetDimensions(dstDims.x, dstDims.y, dstDims.z);
    if (any(srcDims != dstDims))
        return;
    if (any(tid >= dstDims))
        return;
    dstTexture[tid] = srcTexture[tid];
}
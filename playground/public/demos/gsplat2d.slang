import playground;

[playground::RAND(131072)]
RWStructuredBuffer<float> randBuffer;

#define GAUSSIANS_PER_BLOCK 256
#define WG_X 16
#define WG_Y 16

groupshared uint blobs[GAUSSIANS_PER_BLOCK];
groupshared Atomic<uint> blobCount;

typealias ImageCoords = float2;

struct BBox
{
    float2 minVal;
    float2 maxVal;

    __init(float2 minVal, float2 maxVal)
    {
        this.minVal = minVal;
        this.maxVal = maxVal;
    }

    bool intersects(BBox other)
    {
        return !(other.minVal.x > this.maxVal.x
            || other.maxVal.x < this.minVal.x
            || other.maxVal.y < this.minVal.y
            || other.minVal.y > this.maxVal.y);
    }
};

struct SymmetricGaussian2D
{
    float2 center;
    float sigma;
    float3 color;

    static SymmetricGaussian2D load(uint idx)
    {
        uint total = SymmetricGaussian2D.count();
        SymmetricGaussian2D gaussian;
        gaussian.center = float2(randBuffer[total * 0 + idx], randBuffer[total * 1 + idx]);
        gaussian.sigma = abs(float(randBuffer[total * 2 + idx]));
        gaussian.color = float3(randBuffer[total * 3 + idx], randBuffer[total * 4 + idx], randBuffer[total * 5 + idx]);

        // Scale the sigma so the blobs aren't too large
        gaussian.sigma *= 0.04;

        return gaussian;
    }

    static uint count()
    {
        uint elementCount = (uint) 131072;
        return elementCount / 6;
    }

    float4 eval(float2 uv)
    {
        float2 diff = uv - center;
        float dist = dot(diff, diff);
        float weight = exp(-dist / (2.0 * sigma * sigma));
        return float4(color, weight);
    }

    BBox bounds()
    {
        float2 halfSize = float2(sigma, sigma) * 3;
        return BBox(center - halfSize, center + halfSize);
    }
};

struct Maybe<T>
{
    bool valid;
    T value;
};

float4 preMult(float4 pixel)
{
    return float4(pixel.rgb * pixel.a, pixel.a);
}

float4 alphaBlend(float4 pixel, float4 gval)
{
    gval = preMult(gval);
    return float4(
        pixel.rgb + gval.rgb * pixel.a,
        pixel.a * (1 - gval.a));
}

Maybe<float4> transformPixelState(float4 pixel, float4 gval)
{
    var newState = alphaBlend(pixel, gval);

    if (pixel.a < 0.01)
        return Maybe<float4>(false, pixel);

    return Maybe<float4>(true, newState);
}

void bitonicSort(uint localIdx)
{
    uint maxN = blobCount.load();
    for (uint k = 2; k <= GAUSSIANS_PER_BLOCK; k *= 2)
    {
        for (uint j = k / 2; j > 0; j /= 2)
        {
            for (uint i = localIdx; i < GAUSSIANS_PER_BLOCK; i += WG_X * WG_Y)
            {
                uint l = i ^ j;
                if (l > i)
                {
                    if ((((i & k) == 0) && (blobs[i] > blobs[l])) ||
                        (((i & k) != 0) && (blobs[i] < blobs[l])))
                    {
                        // Swap
                        var temp = blobs[i];
                        blobs[i] = blobs[l];
                        blobs[l] = temp;
                    }
                }
            }

            AllMemoryBarrierWithGroupSync();
        }
    }
}

void coarseRasterize(BBox tileBounds, uint localIdx)
{
    SymmetricGaussian2D gaussian;
    uint numGaussians = SymmetricGaussian2D.count();
    for (uint i = localIdx; i < numGaussians; i += (WG_X * WG_Y))
    {
        gaussian = SymmetricGaussian2D.load(i);
        BBox bounds = gaussian.bounds();
        if (bounds.intersects(tileBounds))
        {
            blobs[blobCount++] = i;
        }
    }
}

float4 fineRasterize(float2 uv)
{
    SymmetricGaussian2D gaussian;
    float4 color = float4(0, 0, 0, 1);
    uint count = blobCount.load();
    for (uint i = 0; i < count; i++)
    {
        gaussian = SymmetricGaussian2D.load(blobs[i]);
        var gval = gaussian.eval(uv);
        var newColor = transformPixelState(color, gval);

        if (!newColor.valid)
            return color;

        color = newColor.value;
    }

    return color;
}


float4 imageMain(uint2 dispatchThreadID, int2 screenSize)
{
    uint globalID = dispatchThreadID.x + dispatchThreadID.y * screenSize.x;

    float2 uv = float2(dispatchThreadID) / float2(screenSize);

    uint2 tileID = uint2(dispatchThreadID.x / WG_X, dispatchThreadID.y / WG_Y);

    float2 tileLow = float2(tileID) * float2(WG_X, WG_Y) / float2(screenSize);
    float2 tileHigh = tileLow + (float2(WG_X, WG_Y) / float2(screenSize));

    BBox tileBounds = BBox(tileLow, tileHigh);

    // Short-list blobs that overlap with the current pixel
    if (dispatchThreadID.x % WG_X == 0 && dispatchThreadID.y % WG_Y == 0)
        blobCount = 0;

    AllMemoryBarrierWithGroupSync();

    uint2 localID = dispatchThreadID % uint2(WG_X, WG_Y);
    uint localIdx = localID.x + localID.y * WG_X;

    coarseRasterize(tileBounds, localIdx);
    AllMemoryBarrierWithGroupSync();

    var maxN = blobCount.load();
    for (uint i = localIdx; i < GAUSSIANS_PER_BLOCK; i += (WG_X * WG_Y))
    {
        if (i >= maxN)
            blobs[i] = uint::maxValue;
    }
    AllMemoryBarrierWithGroupSync();

    bitonicSort(localIdx);
    AllMemoryBarrierWithGroupSync();

    float4 color = fineRasterize(uv);

    // Blend with background
    return float4(color.rgb * (1.0 - color.a) + color.a, 1.0);
}
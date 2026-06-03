// Bilateral denoise — spatial-domain weighted average that preserves
// edges by weighting samples both by spatial distance and by intensity
// distance from the centre pixel. The four boundary-handling modes
// (clamp / wrap / reflect / black) and the radius-dependent fast-path
// are the branches that the coverage demo highlights.

public enum BoundaryMode
{
    Clamp = 0,
    Wrap = 1,
    Reflect = 2,
    Black = 3,
}

public float3 sampleWithBoundary(
    RWStructuredBuffer<float4> image,
    int x,
    int y,
    int width,
    int height,
    BoundaryMode mode)
{
    int sx = x;
    int sy = y;

    switch (mode)
    {
    case BoundaryMode::Clamp:
        sx = clamp(x, 0, width - 1);
        sy = clamp(y, 0, height - 1);
        break;
    case BoundaryMode::Wrap:
        sx = ((x % width) + width) % width;
        sy = ((y % height) + height) % height;
        break;
    case BoundaryMode::Reflect:
        if (sx < 0)
            sx = -sx;
        else if (sx >= width)
            sx = 2 * width - sx - 2;
        if (sy < 0)
            sy = -sy;
        else if (sy >= height)
            sy = 2 * height - sy - 2;
        sx = clamp(sx, 0, width - 1);
        sy = clamp(sy, 0, height - 1);
        break;
    case BoundaryMode::Black:
        if (sx < 0 || sx >= width || sy < 0 || sy >= height)
            return float3(0.0, 0.0, 0.0);
        break;
    }

    return image[sy * width + sx].rgb;
}

public float computeLuminance(float3 rgb)
{
    return dot(rgb, float3(0.2126, 0.7152, 0.0722));
}

float spatialWeight(int dx, int dy, float sigma)
{
    float r2 = float(dx * dx + dy * dy);
    return exp(-r2 / (2.0 * sigma * sigma));
}

float rangeWeight(float dl, float sigma)
{
    return exp(-(dl * dl) / (2.0 * sigma * sigma));
}

// Fast 3x3 separable path triggers only when sigmaS < 1.5; the wider
// kernel (general path) is exercised otherwise. Demo input deliberately
// covers both regimes when --mode=exhaustive.
public float3 bilateralDenoisePixel(
    RWStructuredBuffer<float4> input,
    int x,
    int y,
    int width,
    int height,
    float sigmaS,
    float sigmaR,
    BoundaryMode boundary)
{
    float3 centre = sampleWithBoundary(input, x, y, width, height, boundary);
    float centreL = computeLuminance(centre);

    int radius;
    if (sigmaS < 1.5)
    {
        // Fast path — 3x3 kernel
        radius = 1;
    }
    else
    {
        // General path — kernel radius derived from sigmaS
        radius = int(ceil(sigmaS * 2.0));
        if (radius > 7)
            radius = 7;
    }

    float3 acc = float3(0.0, 0.0, 0.0);
    float wsum = 0.0;
    for (int dy = -radius; dy <= radius; ++dy)
    {
        for (int dx = -radius; dx <= radius; ++dx)
        {
            float3 sample = sampleWithBoundary(input, x + dx, y + dy, width, height, boundary);
            float sampleL = computeLuminance(sample);
            float ws = spatialWeight(dx, dy, sigmaS);
            float wr = rangeWeight(sampleL - centreL, sigmaR);
            float w = ws * wr;
            acc += sample * w;
            wsum += w;
        }
    }

    if (wsum > 0.0)
        return acc / wsum;
    return centre;
}

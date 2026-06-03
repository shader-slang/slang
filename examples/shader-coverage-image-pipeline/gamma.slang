// Gamma correction — converts linear radiance to display-encoded
// values. Three encodings, three branches, distinct curves; only one
// arm runs per dispatch.

public enum GammaMode
{
    SRGB = 0,
    Linear = 1,
    Rec709 = 2,
}

float srgbEncodeChannel(float x)
{
    if (x <= 0.0031308)
        return 12.92 * x;
    return 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}

float3 srgbEncode(float3 rgb)
{
    return float3(
        srgbEncodeChannel(rgb.r),
        srgbEncodeChannel(rgb.g),
        srgbEncodeChannel(rgb.b));
}

float rec709EncodeChannel(float x)
{
    if (x < 0.018)
        return 4.5 * x;
    return 1.099 * pow(x, 0.45) - 0.099;
}

float3 rec709Encode(float3 rgb)
{
    return float3(
        rec709EncodeChannel(rgb.r),
        rec709EncodeChannel(rgb.g),
        rec709EncodeChannel(rgb.b));
}

public float3 applyGamma(float3 rgb, GammaMode mode)
{
    switch (mode)
    {
    case GammaMode::SRGB:
        return srgbEncode(rgb);
    case GammaMode::Linear:
        return rgb;
    case GammaMode::Rec709:
        return rec709Encode(rgb);
    default:
        return rgb;
    }
}

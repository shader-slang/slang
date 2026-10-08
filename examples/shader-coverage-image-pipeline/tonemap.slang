// Tone mapping — converts unbounded HDR radiance to displayable
// 0..1 range. The four-way switch on operator is the textbook example
// of branch coverage beating line coverage: line coverage marks
// `applyTonemap` as covered after any operator runs; branch coverage
// shows the three unused arms.

public enum TonemapOperator
{
    Reinhard = 0,
    ACES = 1,
    Hable = 2,
    Uncharted2 = 3,
}

float3 reinhardOp(float3 hdr)
{
    return hdr / (hdr + float3(1.0));
}

// ACES filmic curve approximation (Krzysztof Narkowicz, "ACES Filmic
// Tone Mapping Curve", 2015).
float3 acesOp(float3 hdr)
{
    float3 x = hdr * 0.6;
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float3 hablePartial(float3 x)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 hableOp(float3 hdr)
{
    float3 numer = hablePartial(hdr * 2.0);
    float3 whiteScale = float3(1.0) / hablePartial(float3(11.2));
    return numer * whiteScale;
}

float3 uncharted2Op(float3 hdr)
{
    // John Hable's original Uncharted 2 filmic operator before the
    // 2014 ACES revision. Slightly different curve shape than `hableOp`
    // — kept here as a separate operator so the demo has four distinct
    // arms.
    const float A = 0.22;
    const float B = 0.30;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.01;
    const float F = 0.30;
    const float W = 11.2;
    float3 x = hdr * 2.0;
    float3 result = ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
    float wScale = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
    return result / wScale;
}

public float3 applyTonemap(float3 hdr, TonemapOperator op)
{
    float3 mapped;
    switch (op)
    {
    case TonemapOperator::Reinhard:
        mapped = reinhardOp(hdr);
        break;
    case TonemapOperator::ACES:
        mapped = acesOp(hdr);
        break;
    case TonemapOperator::Hable:
        mapped = hableOp(hdr);
        break;
    case TonemapOperator::Uncharted2:
        mapped = uncharted2Op(hdr);
        break;
    default:
        mapped = hdr;
        break;
    }
    return saturate(mapped);
}

RWStructuredBuffer<float> outputBuffer;

float3 evalBSDF(float3 position)
{
    return float3(0.0f, 0.0f, 0.0f);
}

struct Intersection
{
    float3 position;
    __init(float3 position) { this.position = position; }
}

// Data structure for which differential is automatically computed
struct PathData : IDifferentiable
{
    float3 thp;
    float3 radiance;
    uint length;
    bool is_terminated;
}

[BackwardDifferentiable]
void handleHit(inout PathData path, Intersection isect)
{
    float3 bsdf = evalBSDF(isect.position);
    path.thp *= bsdf;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    PathData pathData;
    DifferentialPair<PathData> pathDataDiff;

    __bwd_diff(handleHit)(pathDataDiff, Intersection(float3(0.0f, 0.0f, 0.0f)));

    var dRadiance = pathDataDiff.getDifferential().radiance;
}

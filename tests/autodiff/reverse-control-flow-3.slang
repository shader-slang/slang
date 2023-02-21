//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<float> outputBuffer;

struct PathState
{
    float f;
    bool terminated;
    bool isHit() { return f != 0.0f; }
    bool isTerminated() { return terminated; }
};

struct PathResult : IDifferentiable
{
    float rs;
}
struct VisibilityQuery
{
    bool test();
}

struct ClosestHitQuery
{
    bool test();
}
void generatePath(uint pathID, out PathState path)
{
    path.f = 1.0f;
    path.terminated = false;
}

[BackwardDifferentiable]
void handleHit(inout PathState path, inout PathResult rs, inout VisibilityQuery vq)
{
    path.terminated = true;
    rs.rs = rs.rs * 5.0f;
}

[BackwardDifferentiable]
void nextHit(inout PathState path, inout PathResult rs, inout ClosestHitQuery cq)
{
    path.f = 1.0f;
    path.terminated = true;
}

[BackwardDifferentiable]
void handleMiss(inout PathState path, inout PathResult rs)
{
    rs.rs = 0.0f;
    path.terminated = true;
}

[BackwardDifferentiable]
bool tracePath(uint pathID, out PathState path, inout PathResult pathRes)
{
    generatePath(pathID, path);

    if (!path.isHit()) return false;

    [ForceUnroll]
    for (int i = 0; i < 3; ++i)
    {
        if (path.isHit())
        {
            VisibilityQuery vq;
            handleHit(path, pathRes, vq);

            if (path.isTerminated()) break;

            ClosestHitQuery chq;
            nextHit(path, pathRes, chq);
        }
        else
        {
            handleMiss(path, pathRes);
        }
    }
    return true;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    {
        PathResult pathRes;
        pathRes.rs = 3.0f;
        PathResult.Differential pathResD;
        pathResD.rs = 1.0f;
        var dpx = diffPair(pathRes, pathResD);
        __bwd_diff(tracePath)(1, dpx);
        outputBuffer[0] = dpx.d.rs; // Expect: 5.0
    }
}

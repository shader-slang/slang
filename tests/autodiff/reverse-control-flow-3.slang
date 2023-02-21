//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer

RWStructuredBuffer<float> outputBuffer;

struct PathState
{
    uint depth;
    bool terminated;

    bool isHit() { return !terminated; }
    bool isTerminated() { return terminated; }
};

struct PathResult : IDifferentiable
{
    float thp;
    float L;
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
    path.terminated = false;
    path.depth = 0;
}

[BackwardDifferentiable]
float lightEval(uint depth)
{
    if (depth == 1)
    {
        return 5.0f;
    }
    else
    {
        return 0.0f;
    }
}

[BackwardDifferentiable]
void handleHit(inout PathState path, inout PathResult rs, inout VisibilityQuery vq)
{
    rs.thp = rs.thp * bsdfEval();

    rs.L = rs.thp * lightEval(path.depth);

    // Decide on next hit
    if (path.depth < 1)
        path.terminated = false;
    else
        path.terminated = true;
}

[BackwardDifferentiable]
float bsdfEval()
{
    return 0.5f;
}

[ForwardDerivativeOf(bsdfEval)]
DifferentialPair<float> __fwd_bsdfEval()
{
    return DifferentialPair<float>(0.5f, 1.0f);
}

[BackwardDerivativeOf(bsdfEval)]
void __bwd_bsdfEval(float dOut)
{
    outputBuffer[3] += dOut;
}

[BackwardDifferentiable]
void nextHit(inout PathState path, inout PathResult rs, inout ClosestHitQuery cq)
{
    path.depth = path.depth + 1;
}

[BackwardDifferentiable]
void handleMiss(inout PathState path, inout PathResult rs)
{
    rs.L = 0.0f;
    path.terminated = true;
}

[BackwardDifferentiable]
bool tracePath(uint pathID, out PathState path, inout PathResult pathRes)
{
    generatePath(pathID, path);

    float thp = pathRes.thp;
    float L = pathRes.L;

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
        pathRes.L = 0.f;
        pathRes.thp = 1.f;

        PathState path;
        tracePath(1, path, pathRes);

        outputBuffer[0] = pathRes.L; // Expect: 1.25
    }

    {
        PathResult pathRes;
        pathRes.L = 0.f;
        pathRes.thp = 1.f;

        PathResult.Differential pathResD;
        pathResD.L = 1.0f;

        PathState path;

        var dpx = diffPair(pathRes, pathResD);
        __fwd_diff(tracePath)(1, path, dpx);

        outputBuffer[1] = dpx.d.L; // Expect: 1.25
        outputBuffer[2] = dpx.p.L; // Expect: 5.0
    }

    {
        PathResult pathRes;
        pathRes.L = 1.f;
        pathRes.thp = 1.f;

        PathResult.Differential pathResD;
        pathResD.L = 1.0f;
        pathResD.thp = 0.f;

        var dpx = diffPair(pathRes, pathResD);
        __bwd_diff(tracePath)(1, dpx); // Expect: 5.0 in outputBuffer[3]
    }
}

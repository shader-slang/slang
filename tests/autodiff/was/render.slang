import was_1d;

[BackwardDifferentiable]
float renderSample(PRNG prng)
{
    float u = no_diff(prng.nextFloat1D());
    
    float leftBound = 0.0;
    float rightBound = 10.0;

    float sample = leftBound * u + rightBound * (1 - u);
    float weight = 1.0/(rightBound - leftBound);

    Intersection isect = warpedIntersect(sample, prng);

    return isect.color * weight;
}

[numthreads(1024, 1, 1)]
float computeMain(uint3 threadIdx : SV_DispatchThreadID,)
{
    PRNG prng = PRNG(threadIdx.x);
    
    float d_color = 1.0;
    __bwd_diff(renderSample)(prng, d_color);
}
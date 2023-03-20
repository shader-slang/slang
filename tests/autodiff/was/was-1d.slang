__exported import scene;
__exported import prng;

float sample1DNormal(float x, PRNG prng)
{
    float u = prng.nextFloat1D();
    float v = prng.nextFloat1D();
    return sqrt(-2 * log(u))*cos(2*M_PI*v);
}

float boundaryTerm(Intersection isect)
{
    if (!isect.isIntersected)
        return 100.0; // Large default value for missed rays.
    
    return ((isect.ls.x1 - x) * (x - isect.ls.x0));
}

[BackwardDifferentiable]
float infinitesimal(float x)
{
    return x - detach(x);
}

[ForwardDifferentiable]
float harmonicWeight(Intersection isect, no_diff Intersection aux_isect)
{
    return 1.0 / ((isect.x - aux_isect.x) ** 2 + no_diff(boundaryTerm(aux_isect)));
}

[BackwardDifferentiable]
Intersection attachToGeometry(Intersection isect)
{
    float leftWt = detach(isect.ls.x1 - isect.x);
    float rightWt = detach(isect.x - isect.x0);

    isect.x = (leftWt * isect.ls.x0 + rightWt * isect.ls.x1);
    return isect;
}

[BackwardDifferentiable]
float warp(Intersection isect, PRNG prng)
{
    float totalWeight = 0.f;
    float totalWarpedPoint = 0.f;

    for (int i = 0; i < 8; i++)
    {
        float y = no_diff(sample1DNormal(prng));

        Intersection aux_isect = intersect(y + isect.x);

        if (aux_isect.is_intersected)
        {
            float wt = harmonicWeight(isect, aux_isect);
            totalWarpedPoint += attachToGeometry(aux_isect, x) * wt;
            totalWeight += wt;
        }
    }

    return totalWarpedPoint / totalWeight;
}

[BackwardDifferentiable]
Intersection warpedIntersect(float x, PRNG prng)
{   
    Intersection isect = intersect(x);

    isect.x = detach(isect.x) + infinitesimal(warp(x, prng));
    isect.color = isect.color * (1 + infinitesimal(__fwd_diff(warp)(diffPair(x, 1.0))));

    return isect;
}
// Positive companion for issue #11285: carry-set updates that arrive through
// loop back-edges must trigger another propagation pass. Otherwise a later
// differentiable call can be missed and E41031 can be lost.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

public struct DirSample
{
    public float distance;
}

[Differentiable]
public float pass_through(float x)
{
    return x;
}

[Differentiable]
public void store_through_loop(float x, no_diff out DirSample sample)
{
    sample = {};

    float v = 0.0;
    [MaxIters(2)]
    for (uint i = 0; i < 2; ++i)
    {
        float r = pass_through(v);
        v = i == 0 ? x : r;
    }

    sample.distance = pass_through(v);
}

RWStructuredBuffer<float> out_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    DirSample s;
    store_through_loop(2.0, s);
    out_buf[0] = s.distance;
}

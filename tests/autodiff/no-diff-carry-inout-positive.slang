// Positive companion for issue #11285: a differentiable call whose carrying
// input flows through an inout parameter must still be considered carrying.
// Otherwise E41031 can be missed when the call result is stored into no_diff
// output state.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost

public struct DirSample
{
    public float distance;
}

[Differentiable]
public float scale_inout(inout float x)
{
    x = x * 2.0;
    return x;
}

[Differentiable]
public void store_through_inout(float x, no_diff out DirSample sample)
{
    sample = {};

    float v = x;
    float r = scale_inout(v);

    // `r` depends on the differentiable input `x` through an inout argument.
    // Storing it into no_diff output state must remain diagnosed.
    sample.distance = r;
}

RWStructuredBuffer<float> out_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    DirSample s;
    store_through_inout(2.0, s);
    out_buf[0] = s.distance;
}

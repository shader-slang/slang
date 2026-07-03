// Positive companion to no-diff-carry-over-call.slang for issue #11285:
// verifies that the auto-diff check still *correctly* emits E41031 when
// a derivative-carrying value is actually stored into a location that
// cannot hold a derivative. We must not over-relax the analysis.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// CHECK: error[E41031]
// CHECK: derivative is lost
// CHECK: sample.distance = sqrt(x);

public struct DirSample
{
    public float distance;
}

[Differentiable]
public void store_through(float x, no_diff out DirSample sample)
{
    sample = {};
    // `x` is a normal (diff-carrying) input. `sqrt(x)` carries a derivative.
    // Storing it into the `no_diff` sample's distance field must still
    // be flagged — this is what makes the diagnostic useful.
    sample.distance = sqrt(x);
}

RWStructuredBuffer<float> out_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    DirSample s;
    store_through(2.0, s);
    out_buf[0] = s.distance;
}

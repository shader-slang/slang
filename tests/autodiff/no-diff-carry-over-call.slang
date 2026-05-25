// Regression test for https://github.com/shader-slang/slang/issues/11285.
//
// A `[Differentiable]` function may legitimately assign to fields of a
// `no_diff out` struct parameter using values derived purely from `no_diff`
// inputs. The auto-diff checker's `isInstCarryingOverDiff` used to mark
// every call to a differentiable function (`sqrt`, `dot`, `max`, …) as
// carrying a non-trivial derivative regardless of whether any argument did,
// which then false-positive-fired `E41031: derivative is lost during
// assignment` at the store. Now a call only carries a derivative when at
// least one argument does.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main

// Compile must succeed (result code = 0) with no `derivative is lost`
// diagnostic and no E41031 error. A trailing positive check for `void main(`
// in the emitted HLSL confirms code generation completed.
// CHECK: result code = 0
// CHECK-NOT: error[E41031]
// CHECK-NOT: derivative is lost
// CHECK: void main(

public struct DirSample
{
    public float3 pos_ws;
    public float distance;
    public float pdf;
}

[Differentiable]
public bool sample_dir(
    no_diff const float3 pos_ws,
    no_diff out DirSample sample,
    out float3 weight)
{
    sample = {};
    weight = float3(0);

    float3 to_light = sample.pos_ws - pos_ws;
    const float dist_sqr = max(1e-30f, dot(to_light, to_light));
    sample.distance = sqrt(dist_sqr);
    sample.pdf = dist_sqr / 1.0f;
    return true;
}

RWStructuredBuffer<float> out_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    DirSample s;
    float3 w;
    sample_dir(float3(0), s, w);
    out_buf[0] = s.distance + s.pdf;
}

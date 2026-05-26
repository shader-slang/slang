// Regression tests for https://github.com/shader-slang/slang/issues/11285.
//
// A `[Differentiable]` function may legitimately assign to fields of a
// `no_diff out` struct parameter using values derived purely from `no_diff`
// inputs. The auto-diff checker's `isInstCarryingOverDiff` used to mark
// every call to a differentiable function (`sqrt`, `dot`, `max`, …) as
// carrying a non-trivial derivative regardless of whether any argument did,
// which then false-positive-fired `E41031: derivative is lost during
// assignment` at the store. The same defect existed in two other places:
//  - block-param phi propagation copied carry-status into `no_diff`-typed
//    block params (e.g. loop params reassigned from a differentiable call),
//    which then leaked into reads of the param in later iterations.
//  - the data-flow fixed-point loop watched only `produceDiffSet`, so
//    a carry-set update arriving via a back-edge call result could miss
//    the next propagation round.

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
    public float3 dir_ws;
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

// Falcor2-style: loop with a `no_diff` block-param reassigned from a
// differentiable constructor call. Phi propagation must not copy carry-set
// membership into the `no_diff`-typed loop param, and the fixed-point loop
// must observe carry-set updates arriving via the back-edge.
public struct Ray : IDifferentiable
{
    public float3 dir;
    public float3 origin;
}

public struct PathState : IDifferentiable
{
    public float3 scatter_dir_ws;
}

[Differentiable]
public void trace_path(
    no_diff Ray ray,
    out float3 color)
{
    PathState path;
    path.scatter_dir_ws = float3(0);
    color = float3(0);

    [MaxIters(2)]
    for (uint depth = 1; depth <= 2; ++depth) {
        DirSample sample = {};
        sample.dir_ws = ray.dir;   // E41031 candidate prior to fix
        // Reassign ray from a differentiable constructor call. The call
        // result is in carryNonTrivialDiffSet (it depends on path's diff
        // fields); the back-edge then attempts to propagate that carry
        // status into the `no_diff` loop param `ray`, which the fix blocks.
        ray = Ray(path.scatter_dir_ws, path.scatter_dir_ws);
        path.scatter_dir_ws = sample.dir_ws;
    }
    color = path.scatter_dir_ws;
}

RWStructuredBuffer<float> out_buf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    DirSample s;
    float3 w;
    sample_dir(float3(0), s, w);

    Ray r = { float3(1, 0, 0), float3(0) };
    float3 c;
    trace_path(r, c);

    out_buf[0] = s.distance + s.pdf + c.x;
}

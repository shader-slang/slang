//TEST:SIMPLE: -target spirv -entry fs -stage fragment -o bwd-diff-member-method-explicit-this.spv

// Regression test for bwd_diff(value.method) after bwd_diff became a static
// associated function. The receiver must be passed explicitly as argument 0.

interface IShape3D
{
    [Differentiable]
    [NoDiffThis]
    float distance(float3 p);
}

struct Sphere : IShape3D
{
    float r;

    [Differentiable]
    [NoDiffThis]
    float distance(float3 p)
    {
        return length(p) - r;
    }
}

float3 gradient3<S : IShape3D>(S shape, float3 p)
{
    var dp = diffPair(p);

    // Generic/interface receiver case: the first argument is the concrete
    // generic receiver `shape`, followed by the original method parameters.
    __bwd_diff(shape.distance)(shape, dp, 1.0);
    return dp.d;
}

[shader("fragment")]
float4 fs(float4 pos : SV_Position) : SV_Target
{
    Sphere s = {1.0};
    var dp = diffPair(pos.xyz);

    // Concrete receiver case: `s.distance` still requires `s` as the
    // explicit receiver for the synthesized static backward derivative.
    __bwd_diff(s.distance)(s, dp, 1.0);
    return float4(dp.d + gradient3(s, pos.xyz), 1.0);
}

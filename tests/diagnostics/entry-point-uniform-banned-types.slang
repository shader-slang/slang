// Verify that types banned as entry-point varyings are still accepted when
// used in uniform positions. Exercises both the uniform-parameter exemption
// path (a `ConstantBuffer<...>` parameter carrying a banned type) and a
// global-scope `ConstantBuffer` for good measure. The varying-type rules
// must not fire for either.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage fragment

struct Params
{
    float x;
    // Banned varying types are fine as fields of a uniform constant buffer.
    DifferentialPair<float> dp;
}

ConstantBuffer<Params> g_params;

[shader("fragment")]
float4 main(ConstantBuffer<Params> p : register(b1)) : SV_Target
{
    return float4(g_params.x + p.x, 0, 0, 0);
}

// The banned-as-varying diagnostics must not fire for uniform positions.
// CHECK-NOT: E38050
// CHECK-NOT: E38051
// CHECK-NOT: not a valid varying type
// CHECK-NOT: not a valid SPIR-V varying type

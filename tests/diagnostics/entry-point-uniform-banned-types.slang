// Verify that types banned as entry-point varyings are still accepted when
// used as uniform parameters (explicit `uniform` modifier, or via wrappers
// such as `ConstantBuffer`). The varying-type rules must only apply to
// parameters that actually participate in inter-stage varying I/O.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage fragment

struct Params
{
    float x;
    vector<bool, 4> flags;
    matrix<uint64_t, 2, 2> m;
}

ConstantBuffer<Params> g_params;

[shader("fragment")]
float4 main(uniform float scale) : SV_Target
{
    return float4(g_params.x * scale, 0, 0, 0);
}

// The SPIR-V-only forbidden varying-type diagnostics must not fire for
// banned types (vector<bool>, matrix<uint64_t>) when they appear only as
// fields of a uniform `ConstantBuffer`, and a uniform parameter must not
// trigger any varying-type diagnostic.
// CHECK-NOT: E38050
// CHECK-NOT: E38051
// CHECK-NOT: not a valid varying type
// CHECK-NOT: not valid as a SPIR-V varying type
// Positive check that we produced valid SPIR-V asm output:
// CHECK: OpCapability

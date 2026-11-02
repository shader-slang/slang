// WGSL's dpdxFine/dpdyFine/dpdxCoarse/dpdyCoarse and fwidthFine/fwidthCoarse take only f32 and
// vecN<f32>. A half argument is reported at compile time instead of emitting WGSL that Tint rejects.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HALF): -target wgsl -entry fragHalf -stage fragment
//DIAGNOSTIC_TEST:SIMPLE(filecheck=HALF2): -target wgsl -entry fragHalf2 -stage fragment

// A generic wrapper called with float compiles with no diagnostic.
//DIAGNOSTIC_TEST:SIMPLE(diag=GEN): -target wgsl -entry fragGeneric -stage fragment

// HALF: WGSL derivative built-ins take only f32 values
[shader("fragment")]
float4 fragHalf(float4 position : SV_Position) : SV_Target
{
    half h = half(position.x);
    return float4(float(ddx_fine(h)), 0, 0, 1);
}

// HALF2: WGSL derivative built-ins take only f32 values
[shader("fragment")]
float4 fragHalf2(float4 position : SV_Position) : SV_Target
{
    half2 h = half2(position.xy);
    return float4(float2(fwidth_coarse(h)), 0, 1);
}

T fineX<T : __BuiltinFloatingPointType>(T v)
{
    return ddx_fine(v);
}

[shader("fragment")]
float4 fragGeneric(float4 position : SV_Position) : SV_Target
{
    return float4(fineX(position.x), 0, 0, 1);
}

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage fragment -entry fragmentMain

// Regression for shader-slang/slang#10802.
//
// A struct that carries indexed `SV_TARGET` semantics but is produced by a helper function --
// rather than appearing directly as the fragment entry point's return type -- still reaches WGSL
// emission. Each of its fields must be assigned its own `@location`. Before the fix the unparsed
// semantic index (-1, with the ordinal still spelled in the name) made every field collapse onto
// `@location(0)`, which WGSL rejects as a duplicate location.

// Helper / non-entry-point struct: this is the case the original bug missed.
// CHECK-DAG asserts struct survival so the @location checks below fail loudly (not vacuously) if
// inlining/DCE ever drops GBuffer.
//CHECK-DAG: struct GBuffer
struct GBuffer
{
    //CHECK-DAG: @location(0) albedo
    float4 albedo : SV_TARGET0;
    //CHECK-DAG: @location(1) normal
    float4 normal : SV_TARGET1;
};

// Entry-point output struct: normalized by varying-parameter legalization (the working control).
struct FragmentColor
{
    //CHECK-DAG: @location(0) color0
    float4 color0 : SV_TARGET0;
    //CHECK-DAG: @location(1) color1
    float4 color1 : SV_TARGET1;
};

GBuffer computeGBuffer()
{
    GBuffer gb;
    gb.albedo = float4(1.0);
    gb.normal = float4(0.0);
    return gb;
}

FragmentColor fragmentMain()
{
    GBuffer gb = computeGBuffer();
    FragmentColor result;
    result.color0 = gb.albedo;
    result.color1 = gb.normal;
    return result;
}

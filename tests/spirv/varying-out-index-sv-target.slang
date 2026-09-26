// Test that fragment-shader color outputs declared with `SV_Target<N>` semantics out of numeric
// order get `location = N` (matching HLSL and the render-target index), not a declaration-order
// location. Regression for https://github.com/shader-slang/slang/issues/11944.

//TEST:SIMPLE(filecheck=CHECK): -stage fragment -entry pmain -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECKGLSL): -stage fragment -entry pmain -target glsl
//TEST:SIMPLE(filecheck=CHECKDIRECT): -stage fragment -entry pmainDirect -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECKDIRECTGLSL): -stage fragment -entry pmainDirect -target glsl
//TEST:SIMPLE(filecheck=CHECKOUT): -stage fragment -entry pmainOut -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECKOUTGLSL): -stage fragment -entry pmainOut -target glsl

struct PixelOutput
{
    float4 colorOut1 : SV_Target1;
    float4 colorOut0 : SV_Target0;
};

// The output declared with `SV_Target1` must land at `Location 1` and the one declared with
// `SV_Target0` at `Location 0`, regardless of declaration order. CHECK-DAG is used for SPIR-V
// because the two `OpDecorate` lines may appear in either order; the declaration order (colorOut1
// then colorOut0) is intentionally the reverse of the location order.
// CHECK-DAG: OpDecorate %{{.*}}colorOut1 Location 1
// CHECK-DAG: OpDecorate %{{.*}}colorOut0 Location 0

// GLSL emits the `layout(location = N)` qualifier on its own line, immediately followed by the
// output declaration, so CHECK-NEXT ties each location to the field that follows it.
// CHECKGLSL: layout(location = 1)
// CHECKGLSL-NEXT: out vec4 {{.*}}colorOut1
// CHECKGLSL: layout(location = 0)
// CHECKGLSL-NEXT: out vec4 {{.*}}colorOut0

[shader("pixel")]
PixelOutput pmain()
{
    PixelOutput output;
    output.colorOut0 = float4(float3(0.0), 1.0);
    output.colorOut1 = float4(float3(1.0), 1.0);
    return output;
}

// A *direct* (non-struct) fragment return declared with `SV_Target1` must also land at
// `Location 1`, not `Location 0`. Here the result value is itself the leaf output, so the fix has
// to survive the entry-point result-layout base rebasing.
// CHECKDIRECT: OpDecorate %{{.*}}pmainDirect Location 1

// CHECKDIRECTGLSL: layout(location = 1)
// CHECKDIRECTGLSL-NEXT: out vec4

[shader("pixel")]
float4 pmainDirect() : SV_Target1
{
    return float4(float3(1.0), 1.0);
}

// The same out-of-order struct delivered through a top-level `out` parameter (the exact shape from
// the issue report) must also honor the SV_Target index, exercising the parameter-binding path
// rather than the entry-point-result path.
// CHECKOUT-DAG: OpDecorate %{{.*}}colorOut1 Location 1
// CHECKOUT-DAG: OpDecorate %{{.*}}colorOut0 Location 0

// CHECKOUTGLSL: layout(location = 1)
// CHECKOUTGLSL-NEXT: out vec4 {{.*}}colorOut1
// CHECKOUTGLSL: layout(location = 0)
// CHECKOUTGLSL-NEXT: out vec4 {{.*}}colorOut0

[shader("pixel")]
void pmainOut(out PixelOutput o)
{
    o.colorOut0 = float4(float3(0.0), 1.0);
    o.colorOut1 = float4(float3(1.0), 1.0);
}

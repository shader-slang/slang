//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage fragment -entry fragmentMain

// A fragment entry point that forwards its varyings to a callable uses the varyings struct in two
// ways. Varying legalization packs the entry-point input into a new struct whose fields carry
// `@location`, while the callable still takes the original struct as an ordinary value. WGSL only
// permits `@interpolate` next to `@location`, so the ordinary struct must not repeat the
// `nointerpolation` field's `@interpolate(flat)`.

struct Varyings
{
    float4 position : SV_Position;
    nointerpolation uint id : ID;
}

struct Shade : IFunc<float, Varyings>
{
    float scale;
    float operator()(Varyings v) { return float(v.id) * scale; }
}

// CHECK-NOT: @interpolate(flat) {{ *}}id_{{[0-9]+}} : u32
// CHECK: @interpolate(flat) @location(0) id_{{[0-9]+}} : u32
// CHECK-NOT: @interpolate(flat) {{ *}}id_{{[0-9]+}} : u32

[shader("fragment")]
float fragmentMain(Varyings v, uniform Shade shade) : SV_Target0
{
    return shade(v);
}

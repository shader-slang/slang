//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute

// On WGSL, `f16tof32` on a `half` vector and `f32tof16_` on a `float` vector are vector
// conversions, not per-component loops (#13765).

// WGSL-NOT: loop
// WGSL: fn computeMain
// WGSL-NOT: loop
// WGSL: vec3<f32>((halfs
// WGSL-NOT: loop
// WGSL: vec3<f16>((floats
// WGSL-NOT: loop

RWStructuredBuffer<half3> halfs;
RWStructuredBuffer<float3> floats;

[numthreads(1, 1, 1)]
void computeMain()
{
    floats[1] = f16tof32(halfs[0]);
    halfs[1] = f32tof16_(floats[0]);
}

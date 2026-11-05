// WGSL has neither `double` nor 16-bit integers, so `copysign` computes `half` and `float`
// arguments, scalar and vector, through `float`.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute

RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = output[0].x;
    half h = half(output[0].y);
    float3 v = output[1].xyz;
    half2 hv = half2(output[1].zw);
    output[2] = float4(copysign(a, -1.0), float(copysign(h, half(-1.0))), float2(copysign(hv, half2(half(-1.0), half(1.0)))));
    output[3] = float4(copysign(v, float3(-1.0, 1.0, -1.0)), 0.0);
}

// WGSL-NOT: u16
// WGSL: fn copysign_float_{{[0-9]+}}( x_{{[0-9]+}} : vec2<f32>
// WGSL: fn copysign_float_{{[0-9]+}}( x_{{[0-9]+}} : f32
// WGSL: : f16) -> f16
// WGSL-NEXT: {
// WGSL-NEXT: return f16(copysign_float_
// WGSL: fn computeMain

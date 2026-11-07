// With `wgsl_chromium_experimental_resource_table`, every `getResource<T>(index)` call is emitted
// inside the expression that uses it, and a function that receives a looked-up texture or sampler
// is inlined into its caller. Tint crashes Chrome's GPU process on two shapes that would otherwise
// appear: a `let` holding a looked-up texture that is sampled with an inline sampler lookup, and a
// looked-up texture passed to a function parameter (https://issues.chromium.org/issues/574322344).
// A global texture passed to a function keeps its parameter. See shader-slang/slang#13641.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table
//TEST:SIMPLE(filecheck=NOTEMP): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table -DNO_GLOBAL_HELPER

// No texture or sampler is bound to a function-scope `let` or `var`, and no function takes one.
// NOTEMP-NOT: {{^ +(let|var) [A-Za-z0-9_]+ : (texture|sampler)}}
// NOTEMP-NOT: {{[(,] *[A-Za-z0-9_]+ : (texture|sampler)}}
// NOTEMP: outputBuffer_0[

// CHECK: enable chromium_experimental_resource_table;
// CHECK-NOT: {{^ +(let|var) [A-Za-z0-9_]+ : (texture|sampler)}}

// Only the function called with a global texture keeps its texture parameter.
// CHECK-NOT: {{[(,] *[A-Za-z0-9_]+ : (texture|sampler)}}
// CHECK: fn sampleGlobal_0({{ *}}t_{{[0-9]+}} : texture_2d<f32>
// CHECK-NOT: {{[(,] *[A-Za-z0-9_]+ : (texture|sampler)}}
// CHECK: fn main(

// A texture used twice, sampled with an inline sampler lookup.
// CHECK: textureDimensions({{.*}}getResource<texture_2d<f32>>(
// CHECK: textureSampleLevel({{.*}}getResource<texture_2d<f32>>({{.*}}getResource<sampler>(

// A helper that forwards the texture to another helper is inlined too.
// CHECK: textureDimensions({{.*}}getResource<texture_2d<f32>>(
// CHECK: textureSampleLevel({{.*}}getResource<texture_2d<f32>>({{.*}}getResource<sampler>(

// A lookup used in a branch and in a loop.
// CHECK: {{^ *}}if(
// CHECK: textureLoad({{.*}}getResource<texture_2d<f32>>(
// CHECK: {{^ *}}loop
// CHECK: textureSampleLevel({{.*}}getResource<texture_2d<f32>>({{.*}}getResource<sampler>(

// The global texture is still passed to its helper.
// CHECK: sampleGlobal_0(gGlobal_0

struct Material
{
    Texture2D<float4>.Handle tex;
    SamplerState.Handle smp;
};

StructuredBuffer<Material> materials;
RWStructuredBuffer<float4> outputBuffer;
Texture2D<float4> gGlobal;
SamplerState gSampler;

float4 sampleWithSize(Texture2D<float4> t, SamplerState s, float2 uv)
{
    uint w, h;
    t.GetDimensions(w, h);
    return t.SampleLevel(s, uv / float2(w, h), 0.0);
}

float4 forward(Texture2D<float4> t, SamplerState s, float2 uv)
{
    return sampleWithSize(t, s, uv) * 0.5;
}

#ifndef NO_GLOBAL_HELPER
[noinline]
float4 sampleGlobal(Texture2D<float4> t, float2 uv)
{
    return t.SampleLevel(gSampler, uv, 0.0);
}
#endif

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Material m = materials[tid.x];
    Texture2D<float4> tex = m.tex;
    SamplerState smp = m.smp;

    float4 r = sampleWithSize(tex, smp, float2(0.5));
    r += forward(materials[tid.x + 1].tex, materials[tid.x + 1].smp, float2(0.25));

    if (r.x > 0.5)
        r += tex.Load(int3(0));

    for (uint i = 0; i < tid.y; i++)
        r += tex.SampleLevel(smp, float2(i, 0), 0.0);

#ifndef NO_GLOBAL_HELPER
    r += sampleGlobal(gGlobal, float2(0.75));
#endif
    outputBuffer[tid.x] = r;
}

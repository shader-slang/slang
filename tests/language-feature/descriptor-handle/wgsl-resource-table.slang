// With `wgsl_chromium_experimental_resource_table`, a `DescriptorHandle<T>` lookup on WGSL becomes a
// `getResource<T>(index)` call into Chromium's experimental resource table: the handle stays a
// `vec2<u32>`, its `.x` is the table index, and no descriptor-heap binding is declared. A looked-up
// texture that is used more than once is looked up again at each use, and `shade` is inlined.
// Without the capability, the output keeps the global descriptor array. See shader-slang/slang#13641.
//
// The resource-table output was checked with Tint from Dawn c943ea32 (`tint file.wgsl`); the Tint
// bundled with slang-test predates the extension, so `wgsl-spirv-asm` cannot validate it here.

//TEST:SIMPLE(filecheck=TABLE): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table
//TEST:SIMPLE(filecheck=NOHEAP): -target wgsl -stage compute -entry main -capability wgsl_chromium_experimental_resource_table
//TEST:SIMPLE(filecheck=HEAP): -target wgsl -stage compute -entry main

// TABLE: enable chromium_experimental_resource_table;
// TABLE-DAG: tex_0 : vec2<u32>
// TABLE-DAG: smp_0 : vec2<u32>
// TABLE-DAG: gTex_0 : vec2<u32>
// TABLE-NOT: fn shade_0
// TABLE: fn main(
// TABLE-NOT: {{^ +(let|var) [A-Za-z0-9_]+ : (texture|sampler)}}
// TABLE: textureSampleLevel({{.*}}getResource<texture_2d<f32>>({{.*}}.x{{.*}}getResource<sampler>({{.*}}smp_0.x{{.*}}textureLoad({{.*}}getResource<texture_2d<f32>>(
// TABLE: textureDimensions({{.*}}getResource<texture_2d<f32>>({{.*}}gTex_0.x
// TABLE: getResource<texture_2d<f32>>({{.*}}heapIndex

// No heap declaration anywhere in the resource-table output.
// NOHEAP-NOT: _slang_resource_heap

// HEAP-NOT: enable chromium_experimental_resource_table;
// HEAP-NOT: getResource
// HEAP: _slang_resource_heap
// HEAP: tex_0 : vec2<u32>
// HEAP: smp_0 : vec2<u32>
// HEAP: gTex_0 : vec2<u32>

struct Material
{
    DescriptorHandle<Texture2D<float4>> tex;
    DescriptorHandle<SamplerState> smp;
};

StructuredBuffer<Material> materials;
RWStructuredBuffer<float4> outputBuffer;
uniform DescriptorHandle<Texture2D<float4>> gTex;
uniform uint heapIndex;

float4 shade(Texture2D<float4> t, SamplerState s, float2 uv)
{
    return t.SampleLevel(s, uv, 0.0);
}

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Material m = materials[tid.x];
    Texture2D<float4> tex = nonuniform(m.tex);
    SamplerState smp = m.smp;
    float4 r = shade(tex, smp, float2(0.5));
    r += tex.Load(int3(0));

    uint w, h;
    gTex.GetDimensions(w, h);
    r += float(w + h);

    // SM 6.6 direct heap indexing goes through the same lookup.
    Texture2D<float4> direct = ResourceDescriptorHeap[heapIndex];
    r += direct.Load(int3(1, 1, 0));

    outputBuffer[tid.x] = r;
}

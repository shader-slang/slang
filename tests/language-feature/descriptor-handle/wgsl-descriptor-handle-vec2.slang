// On WGSL, a `DescriptorHandle<T>` is emitted as a `vec2<u32>` wherever it appears (a uniform, a
// function parameter, a local), which matches its layout and reflection. Before, the emitter wrote
// the resource type instead, which is invalid in a uniform block or a storage-buffer struct.
// See shader-slang/slang#13641.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry main

// CHECK-NOT: : texture_2d<f32>,
// CHECK: gTex_0 : vec2<u32>
// CHECK: fn f_0({{ *}}h_0 : vec2<u32>)
// CHECK: fn main(
// CHECK: var local_{{[0-9]+}} : vec2<u32>;
// CHECK: f_0(

RWStructuredBuffer<float4> outputBuffer;
uniform Texture2D<float4>.Handle gTex;
uniform Texture2D<float4>.Handle gTex2;
uniform uint pick;

[noinline]
float4 f(Texture2D<float4>.Handle h)
{
    return h.Load(int3(0));
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Texture2D<float4>.Handle local = gTex;
    if (pick != 0)
        local = gTex2;
    outputBuffer[tid.x] = f(local);
}

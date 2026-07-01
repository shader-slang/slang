// Regression test for https://github.com/shader-slang/slang/issues/11825
//
// Warnings E31106 / E31107 ("special type leaks from a parameter group") must NOT fire for a
// compiler-SYNTHESIZED entry-point uniform parameter group. Here the entry point mixes resource
// parameters with a `uniform` ordinary parameter (`dim`); the compiler collects these into an
// implicit `ConstantBuffer<EntryPointParams>` (see CollectEntryPointUniformParams). The user did
// not author that grouping, so the textures going to separate binding slots is expected and there
// is nothing for the user to restructure -- the warning is spurious in this synthesized case.
//
// User-authored cbuffer / ConstantBuffer<S> / push_constants that mix resources + ordinary data
// still warn WITH a source location -- those are guarded by
// parameter-group-special-type-leak-location.slang and rtas-cbuff-leak.slang.

//TEST:SIMPLE(filecheck=CHECKSPV):-target spirv -entry testMain -stage compute
//TEST:SIMPLE(filecheck=CHECKHLSL):-target hlsl -entry testMain -stage compute

// CHECKSPV-NOT: E31106
// CHECKSPV-NOT: E31107
// CHECKSPV: OpEntryPoint

// CHECKHLSL-NOT: E31106
// CHECKHLSL-NOT: E31107
// CHECKHLSL: numthreads

[shader("compute")]
[numthreads(1, 1, 1)]
void testMain(
    uint3 tid : SV_DispatchThreadID,
    Texture2D<float4> texture,
    RWTexture2D<float4> rw_texture,
    uniform int2 dim)
{
    if (all(tid.xy < dim))
        rw_texture[tid.xy] = texture[tid.xy];
}

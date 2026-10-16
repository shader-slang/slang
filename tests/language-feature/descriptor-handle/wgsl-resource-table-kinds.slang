// Every kind that Chromium's WGSL resource table holds (Dawn `wgsl.def`: sampled, depth and
// multisampled textures, `sampler` and `sampler_comparison`) maps to `getResource<T>`.
// See shader-slang/slang#13641. Checked with Tint from Dawn c943ea32.

//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage fragment -entry fragMain -capability wgsl_chromium_experimental_resource_table

// CHECK: enable chromium_experimental_resource_table;
// CHECK-DAG: getResource<texture_1d<f32>>
// CHECK-DAG: getResource<texture_2d<f32>>
// CHECK-DAG: getResource<texture_2d<u32>>
// CHECK-DAG: getResource<texture_2d<i32>>
// CHECK-DAG: getResource<texture_2d_array<f32>>
// CHECK-DAG: getResource<texture_3d<f32>>
// CHECK-DAG: getResource<texture_cube<f32>>
// CHECK-DAG: getResource<texture_cube_array<f32>>
// CHECK-DAG: getResource<texture_multisampled_2d<f32>>
// CHECK-DAG: getResource<texture_depth_2d>
// CHECK-DAG: getResource<texture_depth_2d_array>
// CHECK-DAG: getResource<texture_depth_cube>
// CHECK-DAG: getResource<texture_depth_cube_array>
// CHECK-DAG: getResource<texture_depth_multisampled_2d>
// CHECK-DAG: getResource<sampler>
// CHECK-DAG: getResource<sampler_comparison>

struct Handles
{
    Texture1D<float4>.Handle t1d;
    Texture2D<float4>.Handle t2d;
    Texture2D<uint4>.Handle t2dU;
    Texture2D<int4>.Handle t2dI;
    Texture2DArray<float4>.Handle t2dArray;
    Texture3D<float4>.Handle t3d;
    TextureCube<float4>.Handle tCube;
    TextureCubeArray<float4>.Handle tCubeArray;
    Texture2DMS<float4>.Handle t2dMS;
    DepthTexture2D.Handle d2d;
    DepthTexture2DArray.Handle d2dArray;
    DepthTextureCube.Handle dCube;
    DepthTextureCubeArray.Handle dCubeArray;
    DepthTexture2DMS.Handle d2dMS;
    SamplerState.Handle s;
    SamplerComparisonState.Handle sc;
};
ConstantBuffer<Handles> h;

[shader("fragment")]
float4 fragMain(float2 uv : TEXCOORD0) : SV_Target
{
    float4 r = h.t1d.Sample(h.s, uv.x);
    r += h.t2d.Sample(h.s, uv);
    r += float4(h.t2dU.Load(int3(0)));
    r += float4(h.t2dI.Load(int3(0)));
    r += h.t2dArray.Sample(h.s, float3(uv, 1));
    r += h.t3d.Sample(h.s, float3(uv, 0.5));
    r += h.tCube.Sample(h.s, float3(uv, 1));
    r += h.tCubeArray.Sample(h.s, float4(uv, 1, 0));
    r += h.t2dMS.Load(int2(0), 0);
    r += h.d2d.SampleCmp(h.sc, uv, 0.5);
    r += h.d2dArray.SampleCmp(h.sc, float3(uv, 1), 0.5);
    r += h.dCube.SampleCmp(h.sc, float3(uv, 1), 0.5);
    r += h.dCubeArray.SampleCmp(h.sc, float4(uv, 1, 0), 0.5);
    r += h.d2dMS.Load(int2(0), 0);
    return r;
}

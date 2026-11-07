//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute -warnings-as-errors 31105

// The WGSL `texture_formats_tier1` language extension adds 23 storage texel formats.
// Each one must be emitted under its own WGSL name rather than replaced by `rgba32float`
// (with warning E31105), so the declaration matches the texture bound at run time.
// GitHub issue: https://github.com/shader-slang/slang/issues/13768

// CHECK-DAG: var t_r8{{_[0-9]+}} : texture_storage_2d<r8unorm, write>
// CHECK-DAG: var t_r8_snorm{{_[0-9]+}} : texture_storage_2d<r8snorm, write>
// CHECK-DAG: var t_r8ui{{_[0-9]+}} : texture_storage_2d<r8uint, write>
// CHECK-DAG: var t_r8i{{_[0-9]+}} : texture_storage_2d<r8sint, write>
// CHECK-DAG: var t_rg8{{_[0-9]+}} : texture_storage_2d<rg8unorm, write>
// CHECK-DAG: var t_rg8_snorm{{_[0-9]+}} : texture_storage_2d<rg8snorm, write>
// CHECK-DAG: var t_rg8ui{{_[0-9]+}} : texture_storage_2d<rg8uint, write>
// CHECK-DAG: var t_rg8i{{_[0-9]+}} : texture_storage_2d<rg8sint, write>
// CHECK-DAG: var t_r16{{_[0-9]+}} : texture_storage_2d<r16unorm, write>
// CHECK-DAG: var t_r16_snorm{{_[0-9]+}} : texture_storage_2d<r16snorm, write>
// CHECK-DAG: var t_r16ui{{_[0-9]+}} : texture_storage_2d<r16uint, write>
// CHECK-DAG: var t_r16i{{_[0-9]+}} : texture_storage_2d<r16sint, write>
// CHECK-DAG: var t_r16f{{_[0-9]+}} : texture_storage_2d<r16float, write>
// CHECK-DAG: var t_rg16{{_[0-9]+}} : texture_storage_2d<rg16unorm, write>
// CHECK-DAG: var t_rg16_snorm{{_[0-9]+}} : texture_storage_2d<rg16snorm, write>
// CHECK-DAG: var t_rg16ui{{_[0-9]+}} : texture_storage_2d<rg16uint, write>
// CHECK-DAG: var t_rg16i{{_[0-9]+}} : texture_storage_2d<rg16sint, write>
// CHECK-DAG: var t_rg16f{{_[0-9]+}} : texture_storage_2d<rg16float, write>
// CHECK-DAG: var t_rgba16{{_[0-9]+}} : texture_storage_2d<rgba16unorm, write>
// CHECK-DAG: var t_rgba16_snorm{{_[0-9]+}} : texture_storage_2d<rgba16snorm, write>
// CHECK-DAG: var t_rgb10_a2{{_[0-9]+}} : texture_storage_2d<rgb10a2unorm, write>
// CHECK-DAG: var t_rgb10_a2ui{{_[0-9]+}} : texture_storage_2d<rgb10a2uint, write>
// CHECK-DAG: var t_r11f_g11f_b10f{{_[0-9]+}} : texture_storage_2d<rg11b10ufloat, write>

// A read-write r16f texture keeps `read_write` and names its own format. r16float is one
// of the tier1 formats that allow read_write, and only with the WebGPU
// `texture-formats-tier2` feature.
// CHECK-DAG: var rw_r16f{{_[0-9]+}} : texture_storage_2d<r16float, read_write>

[format("r8")] WTexture2D<float> t_r8;
[format("r8_snorm")] WTexture2D<float> t_r8_snorm;
[format("r8ui")] WTexture2D<uint> t_r8ui;
[format("r8i")] WTexture2D<int> t_r8i;
[format("rg8")] WTexture2D<float2> t_rg8;
[format("rg8_snorm")] WTexture2D<float2> t_rg8_snorm;
[format("rg8ui")] WTexture2D<uint2> t_rg8ui;
[format("rg8i")] WTexture2D<int2> t_rg8i;
[format("r16")] WTexture2D<float> t_r16;
[format("r16_snorm")] WTexture2D<float> t_r16_snorm;
[format("r16ui")] WTexture2D<uint> t_r16ui;
[format("r16i")] WTexture2D<int> t_r16i;
[format("r16f")] WTexture2D<float> t_r16f;
[format("rg16")] WTexture2D<float2> t_rg16;
[format("rg16_snorm")] WTexture2D<float2> t_rg16_snorm;
[format("rg16ui")] WTexture2D<uint2> t_rg16ui;
[format("rg16i")] WTexture2D<int2> t_rg16i;
[format("rg16f")] WTexture2D<float2> t_rg16f;
[format("rgba16")] WTexture2D<float4> t_rgba16;
[format("rgba16_snorm")] WTexture2D<float4> t_rgba16_snorm;
[format("rgb10_a2")] WTexture2D<float4> t_rgb10_a2;
[format("rgb10_a2ui")] WTexture2D<uint4> t_rgb10_a2ui;
[format("r11f_g11f_b10f")] WTexture2D<float3> t_r11f_g11f_b10f;

[format("r16f")] RWTexture2D<float> rw_r16f;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    t_r8.Store(id.xy, 0.5);
    t_r8_snorm.Store(id.xy, -0.5);
    t_r8ui.Store(id.xy, 1);
    t_r8i.Store(id.xy, -1);
    t_rg8.Store(id.xy, float2(0.5, 0.25));
    t_rg8_snorm.Store(id.xy, float2(-0.5, 0.25));
    t_rg8ui.Store(id.xy, uint2(1, 2));
    t_rg8i.Store(id.xy, int2(-1, 2));
    t_r16.Store(id.xy, 0.5);
    t_r16_snorm.Store(id.xy, -0.5);
    t_r16ui.Store(id.xy, 1);
    t_r16i.Store(id.xy, -1);
    t_r16f.Store(id.xy, 0.5);
    t_rg16.Store(id.xy, float2(0.5, 0.25));
    t_rg16_snorm.Store(id.xy, float2(-0.5, 0.25));
    t_rg16ui.Store(id.xy, uint2(1, 2));
    t_rg16i.Store(id.xy, int2(-1, 2));
    t_rg16f.Store(id.xy, float2(0.5, 0.25));
    t_rgba16.Store(id.xy, float4(0.5, 0.25, 0.125, 1.0));
    t_rgba16_snorm.Store(id.xy, float4(-0.5, 0.25, 0.125, 1.0));
    t_rgb10_a2.Store(id.xy, float4(0.5, 0.25, 0.125, 1.0));
    t_rgb10_a2ui.Store(id.xy, uint4(1, 2, 3, 1));
    t_r11f_g11f_b10f.Store(id.xy, float3(0.5, 0.25, 0.125));
    rw_r16f[id.xy] = rw_r16f[id.xy] + 0.5;
}

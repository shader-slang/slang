// Dual-source blending: `[[vk::location(0), vk::index(1)]]` is WGSL's @location(0) @blend_src(1)
// under `enable dual_source_blending;` (it used to become a second color target, @location(1)).
// Metal: [[color(0), index(1)]]. Plain multiple render targets keep their locations.
//TEST:SIMPLE(filecheck=DUAL): -target wgsl -stage fragment -entry dualMain
//TEST:SIMPLE(filecheck=MRT): -target wgsl -stage fragment -entry mrtMain
//TEST:SIMPLE(filecheck=MTL): -target metal -stage fragment -entry dualMain
//TEST:SIMPLE(filecheck=NOSEM): -target wgsl -stage fragment -entry noSemanticMain
//TEST:SIMPLE(filecheck=MTL-NOSEM): -target metal -stage fragment -entry noSemanticMain
//TEST:SIMPLE(filecheck=REORDER): -target wgsl -stage fragment -entry reorderedMain
//TEST:SIMPLE(filecheck=MTL-REORDER): -target metal -stage fragment -entry reorderedMain
//TEST:SIMPLE(filecheck=MIXED): -target wgsl -stage fragment -entry mixedMain
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage fragment -entry dualMain
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage fragment -entry mrtMain

// SPV: OpEntryPoint

struct DualOut
{
    [[vk::location(0), vk::index(0)]] float4 color : SV_Target0;
    [[vk::location(0), vk::index(1)]] float4 blend : SV_Target1;
};

// DUAL: enable dual_source_blending;
// DUAL: @location(0) @blend_src(0) color_{{[0-9]+}} : vec4<f32>
// DUAL: @location(0) @blend_src(1) blend_{{[0-9]+}} : vec4<f32>
// MTL: float4 color_{{[0-9]+}} {{\[\[}}color(0), index(0){{\]\]}};
// MTL: float4 blend_{{[0-9]+}} {{\[\[}}color(0), index(1){{\]\]}};
DualOut dualMain(float4 p: SV_Position)
{
    DualOut o;
    o.color = p;
    o.blend = p * 0.5;
    return o;
}

struct MrtOut
{
    float4 a : SV_Target0;
    float4 b : SV_Target1;
};

// MRT-NOT: blend_src
// MRT: @location(0) a_{{[0-9]+}} : vec4<f32>
// MRT: @location(1) b_{{[0-9]+}} : vec4<f32>
MrtOut mrtMain(float4 p: SV_Position)
{
    MrtOut o;
    o.a = p;
    o.b = p * 0.5;
    return o;
}

// Outputs with only vk::location / vk::index and no semantic (#8003).
struct NoSemanticOut
{
    [[vk::location(0), vk::index(0)]] float4 col0;
    [[vk::location(0), vk::index(1)]] float4 col1;
};

// NOSEM: enable dual_source_blending;
// NOSEM: @location(0) @blend_src(0) col0_{{[0-9]+}} : vec4<f32>
// NOSEM: @location(0) @blend_src(1) col1_{{[0-9]+}} : vec4<f32>
// MTL-NOSEM: float4 col0_{{[0-9]+}} {{\[\[}}color(0), index(0){{\]\]}};
// MTL-NOSEM: float4 col1_{{[0-9]+}} {{\[\[}}color(0), index(1){{\]\]}};
NoSemanticOut noSemanticMain(float4 p: SV_Position)
{
    NoSemanticOut o;
    o.col0 = p;
    o.col1 = p * 0.5;
    return o;
}

// The index(1) output comes first and both use SV_Target0, so legalization renames the second
// semantic. Both outputs must still be at location 0.
struct ReorderedOut
{
    [[vk::location(0), vk::index(1)]] float4 blend : SV_Target0;
    [[vk::location(0), vk::index(0)]] float4 color : SV_Target0;
};

// REORDER: enable dual_source_blending;
// REORDER: @location(0) @blend_src(1) blend_{{[0-9]+}} : vec4<f32>
// REORDER: @location(0) @blend_src(0) color_{{[0-9]+}} : vec4<f32>
// MTL-REORDER: float4 blend_{{[0-9]+}} {{\[\[}}color(0), index(1){{\]\]}};
// MTL-REORDER: float4 color_{{[0-9]+}} {{\[\[}}color(0), index(0){{\]\]}};
ReorderedOut reorderedMain(float4 p: SV_Position)
{
    ReorderedOut o;
    o.blend = p;
    o.color = p * 0.5;
    return o;
}

// A dual-source pair next to an ordinary output at another location.
struct MixedOut
{
    [[vk::location(0), vk::index(0)]] float4 color : SV_Target0;
    [[vk::location(0), vk::index(1)]] float4 blend : SV_Target1;
    [[vk::location(1)]] float4 other : SV_Target2;
};

// MIXED: @location(0) @blend_src(0) color_{{[0-9]+}} : vec4<f32>
// MIXED: @location(0) @blend_src(1) blend_{{[0-9]+}} : vec4<f32>
// MIXED-NOT: blend_src
// MIXED: other_{{[0-9]+}} : vec4<f32>
MixedOut mixedMain(float4 p: SV_Position)
{
    MixedOut o;
    o.color = p;
    o.blend = p * 0.5;
    o.other = p * 0.25;
    return o;
}

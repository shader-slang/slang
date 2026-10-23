//TEST:SIMPLE(filecheck=NESTED): -target spirv-asm -stage fragment -entry nestedMain
//TEST:SIMPLE(filecheck=WNESTED): -target wgsl -stage fragment -entry nestedMain
//TEST:SIMPLE(filecheck=SPARSE): -target spirv-asm -stage fragment -entry sparseMain
//TEST:SIMPLE(filecheck=DUAL): -target spirv-asm -stage fragment -entry dualMain
//TEST:SIMPLE(filecheck=RESULT): -target spirv-asm -stage fragment -entry resultMain

// `SV_Target<N>` is an absolute location: a struct placing fields by it sits at location 0,
// and overlap is checked per leaf.

struct Inner
{
    float4 a : SV_Target1;
    float4 b : SV_Target2;
};
struct NestedOutput
{
    float4 c : SV_Target0;
    Inner nested;
};

// NESTED-DAG: OpDecorate %entryPointParam_nestedMain_c Location 0
// NESTED-DAG: OpDecorate %entryPointParam_nestedMain_nested_a Location 1
// NESTED-DAG: OpDecorate %entryPointParam_nestedMain_nested_b Location 2
// WNESTED-DAG: @location(0) c{{.*}} : vec4<f32>
// WNESTED-DAG: @location(1) a{{.*}} : vec4<f32>
// WNESTED-DAG: @location(2) b{{.*}} : vec4<f32>
NestedOutput nestedMain()
{
    return { float4(1), { float4(2), float4(3) } };
}

struct SparseInner
{
    [[vk::location(0)]] float4 a : SV_Target0;
    [[vk::location(2)]] float4 b : SV_Target2;
};
struct SparseOutput
{
    [[vk::location(0)]] SparseInner inner;
    [[vk::location(1)]] float4 c : SV_Target1;
};

// SPARSE-DAG: OpDecorate %entryPointParam_sparseMain_inner_a Location 0
// SPARSE-DAG: OpDecorate %entryPointParam_sparseMain_inner_b Location 2
// SPARSE-DAG: OpDecorate %entryPointParam_sparseMain_c Location 1
SparseOutput sparseMain()
{
    return { { float4(1), float4(2) }, float4(3) };
}

struct DualInner
{
    [[vk::location(0), vk::index(1)]] float4 a : SV_Target0;
};
struct DualOutput
{
    [[vk::location(0)]] DualInner inner;
    [[vk::location(0)]] float4 b : SV_Target0;
};

// DUAL-DAG: OpDecorate %entryPointParam_dualMain_inner_a Location 0
// DUAL-DAG: OpDecorate %entryPointParam_dualMain_inner_a Index 1
// DUAL-DAG: OpDecorate %entryPointParam_dualMain_b Location 0
DualOutput dualMain()
{
    return { { float4(1) }, float4(2) };
}

struct ResultOutput
{
    float4 a : SV_Target0;
};

// RESULT-DAG: OpDecorate %entryPointParam_resultMain_a Location 0
// RESULT-DAG: OpDecorate %b Location 1
ResultOutput resultMain(out float4 b : SV_Target1)
{
    b = float4(1);
    return { float4(2) };
}

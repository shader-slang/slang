//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib
//TEST:SIMPLE(filecheck=WGSL): -target wgsl
//TEST:SIMPLE(filecheck=WGSLSPIRV): -target wgsl-spirv-asm
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm

// An aggregate declared `SV_Target1` takes targets 1 and 2 for its two members.
// Overlapping targets are diagnosed (tests/diagnostics/overlapping-sv-target.slang).

struct NestedReturn
{
    float4 debug1;
    float4 debug2;
};

struct Output
{
    float4 Diffuse : SV_Target0;
    NestedReturn val : SV_Target1;
    float4 Material : SV_Target3;
}

// METALLIB: define {{.*}} @fragmentMain
// METAL-DAG: color(0)
// METAL-DAG: color(1)
// METAL-DAG: color(2)
// METAL-DAG: color(3)

// WGSLSPIRV: %fragmentMain = OpFunction
// WGSL-DAG: @location(0) Diffuse
// WGSL-DAG: @location(1) debug1
// WGSL-DAG: @location(2) debug2
// WGSL-DAG: @location(3) Material

// SPIRV-DAG: OpDecorate %{{.*}}Diffuse Location 0
// SPIRV-DAG: OpDecorate %{{.*}}debug1 Location 1
// SPIRV-DAG: OpDecorate %{{.*}}debug2 Location 2
// SPIRV-DAG: OpDecorate %{{.*}}Material Location 3

[shader("fragment")]
Output fragmentMain()
{
    return { float4(1), {float4(2), float4(2)}, float4(3) };
}

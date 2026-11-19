//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry first -entry second
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry first -entry second

// Entry points returning one struct under different outer `SV_Target<N>`s give its field
// different locations.

struct Output
{
    float4 color;
};

// WGSL: @location(1) color{{.*}} : vec4<f32>
// WGSL: @location(2) color{{.*}} : vec4<f32>
// SPIRV-DAG: OpDecorate %entryPointParam_first_color Location 1
// SPIRV-DAG: OpDecorate %entryPointParam_second_color Location 2

[shader("fragment")]
Output first() : SV_Target1
{
    return { float4(1) };
}

[shader("fragment")]
Output second() : SV_Target2
{
    return { float4(2) };
}

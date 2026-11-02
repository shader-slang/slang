// A fragment shader's non-struct return value keeps its system-value semantic: SV_Depth and the
// conservative-depth forms write depth, SV_Coverage writes the sample mask, and SV_TargetN writes
// render target N (it used to be render target 0 for all of them).
//TEST:SIMPLE(filecheck=DEPTH): -target wgsl -stage fragment -entry depthMain
//TEST:SIMPLE(filecheck=GE): -target wgsl -stage fragment -entry depthGreaterMain
//TEST:SIMPLE(filecheck=LE): -target wgsl -stage fragment -entry depthLessMain
//TEST:SIMPLE(filecheck=COV): -target wgsl -stage fragment -entry coverageMain
//TEST:SIMPLE(filecheck=T1): -target wgsl -stage fragment -entry target1Main
//TEST:SIMPLE(filecheck=T0): -target wgsl -stage fragment -entry target0Main
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage fragment -entry depthGreaterMain
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage fragment -entry target1Main
//TEST:SIMPLE(filecheck=MTL-DEPTH): -target metal -stage fragment -entry depthMain
//TEST:SIMPLE(filecheck=MTL-GE): -target metal -stage fragment -entry depthGreaterMain
//TEST:SIMPLE(filecheck=MTL-T1): -target metal -stage fragment -entry target1Main

// SPV: OpEntryPoint

// DEPTH: @builtin(frag_depth) output_{{[0-9]+}} : f32
// MTL-DEPTH: float output_{{[0-9]+}} {{\[\[}}depth(any){{\]\]}};
float depthMain(float4 p: SV_Position) : SV_Depth
{
    return p.z * 0.5;
}

// GE: @builtin(frag_depth) output_{{[0-9]+}} : f32
// MTL-GE: float output_{{[0-9]+}} {{\[\[}}depth(greater){{\]\]}};
float depthGreaterMain(float4 p: SV_Position) : SV_DepthGreaterEqual
{
    return p.z + 0.25;
}

// LE: @builtin(frag_depth) output_{{[0-9]+}} : f32
float depthLessMain(float4 p: SV_Position) : SV_DepthLessEqual
{
    return p.z * 0.25;
}

// COV: @builtin(sample_mask) output_{{[0-9]+}} : u32
uint coverageMain(float4 p: SV_Position) : SV_Coverage
{
    return 1;
}

// T1: @location(1) output_{{[0-9]+}} : vec4<f32>
// MTL-T1: float4 output_{{[0-9]+}} {{\[\[}}color(1){{\]\]}};
float4 target1Main(float4 p: SV_Position) : SV_Target1
{
    return p;
}

// T0: @location(0) output_{{[0-9]+}} : vec4<f32>
float4 target0Main(float4 p: SV_Position) : SV_Target
{
    return p;
}

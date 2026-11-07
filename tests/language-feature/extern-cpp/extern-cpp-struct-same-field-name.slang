//TEST:SIMPLE(filecheck=CPP): -target hpp -entry cppEntryPoint -stage compute -line-directive-mode none -DHOST_API
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage fragment -line-directive-mode none
//TEST:SIMPLE(filecheck=METAL): -target metal -entry main -stage fragment -line-directive-mode none
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage fragment -line-directive-mode none

// The fields of an `__extern_cpp` struct keep their source names only in C++
// and CUDA source output (https://github.com/shader-slang/slang/issues/14028).
// Two structs that share a field name keep distinct fields there. GPU targets
// keep uniqued names: Metal and WGSL flatten both structs into one stage-input
// struct, and `sample` is reserved in GLSL-family languages.

//CPP: struct Left
//CPP-NEXT: {
//CPP-NEXT: float x;
//CPP-NEXT: float sample;
//CPP-NEXT: };
//CPP: struct Right
//CPP-NEXT: {
//CPP-NEXT: float x;
//CPP-NEXT: };

//HLSL: struct Left
//HLSL-NEXT: {
//HLSL-NEXT: float x_{{[0-9]+}} : COLOR0;
//HLSL-NEXT: float sample_{{[0-9]+}} : COLOR1;
//HLSL-NEXT: };
//HLSL: struct Right
//HLSL-NEXT: {
//HLSL-NEXT: float x_{{[0-9]+}} : TEXCOORD0;
//HLSL-NEXT: };

//METAL: float x_[[X0:[0-9]+]] {{\[\[}}user(COLOR)]];
//METAL: float sample_{{[0-9]+}} {{\[\[}}user(COLOR_1)]];
//METAL-NOT: float x_[[X0]] {{\[\[}}
//METAL: float x_{{[0-9]+}} {{\[\[}}user(TEXCOORD)]];

//WGSL: @location(0) x_[[X0:[0-9]+]] : f32,
//WGSL: @location(1) sample_{{[0-9]+}} : f32,
//WGSL-NOT: x_[[X0]] :
//WGSL: @location(2) x_{{[0-9]+}} : f32,

__extern_cpp struct Left
{
    float x : COLOR0;
    float sample : COLOR1;
}

__extern_cpp struct Right
{
    float x : TEXCOORD0;
}

float4 main(Left l, Right r) : SV_Target
{
    return float4(l.x, r.x, l.sample, 1);
}

#ifdef HOST_API
export __extern_cpp float sumFields(Left l, Right r)
{
    return l.x + l.sample + r.x;
}

[numthreads(1, 1, 1)]
void cppEntryPoint() {}
#endif

// Check that a never-written combined texture-sampler passed whole to a function compiles on
// targets that do not build a combined value from a texture and a sampler. The C++ target lowers
// the combined value to a structure, GLSL keeps it as one `sampler2D`, and direct SPIR-V removes
// the sample through it.

//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute -O0
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute -O0
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry main -stage compute -O0

// CPP: struct [[COMBINED:Sampler2D_[0-9]+]]
// CPP: [[COMBINED]] {{[A-Za-z0-9_]+}} = {};

// GLSL: sampleWith_0(sampler2D
// GLSL: sampleWith_0(

// SPIRV: OpEntryPoint
// SPIRV-NOT: OpTypeSampledImage

RWStructuredBuffer<float4> outputBuffer;

[noinline]
float4 sampleWith(Sampler2D<float4> combined)
{
    return combined.SampleLevel(float2(0), 0);
}

[numthreads(1, 1, 1)]
void main()
{
    Sampler2D<float4> combined;
    outputBuffer[0] = sampleWith(combined);
}

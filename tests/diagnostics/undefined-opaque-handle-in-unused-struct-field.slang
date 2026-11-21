//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -entry main -stage compute

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COPY,exit-code=0):-target spirv -entry copyMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COPY,exit-code=0):-target hlsl -entry copyMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_SPIRV,exit-code=0):-target spirv -entry usedMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_HLSL):-target hlsl -entry usedMain -stage compute -Wno-using-uninitialized-variable

// An undefined value of a struct type that contains an opaque handle decomposes during type
// legalization into one undefined value per field, so the two spellings of the same program
// behave alike: `SampleArgs args; getScale(args);` compiles as if the user had written the
// fields separately. An opaque field that is never read becomes a dead leaf that DCE removes,
// leaving only the uninitialized-variable warning (issue #14682: this used to be a fatal
// E56003 on pure type containment, on every target, even for a bare copy with no call).

struct SampleArgs
{
    Texture2D<float4> texture;
    float4 scale;
};

RWStructuredBuffer<float4> output;

float4 getScale(SampleArgs args)
{
    return args.scale;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    SampleArgs args;
    output[0] = getScale(args);
//CHECK:           ^ use of uninitialized variable
//CHECK:           ^ use of uninitialized variable 'args'
}

// Even a bare struct-to-local copy with no call keeps only the warning: the copied texture
// leaf is never read, so it is dead code.
[shader("compute")]
[numthreads(1, 1, 1)]
void copyMain()
{
    SampleArgs a;
    SampleArgs b = a;
    output[0] = b.scale;
}

// A genuinely-used texture field keeps the undefined-opaque-handle machinery of #14342: on
// SPIR-V the recognized access is rewritten to an undefined result and the handle becomes
// dead, so the program still compiles.
[shader("compute")]
[numthreads(1, 1, 1)]
void usedMain()
{
    SampleArgs args;
    output[0] = args.texture.Load(int3(0, 0, 0));
}

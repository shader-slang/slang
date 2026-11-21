//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target hlsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target glsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target wgsl -entry main -stage compute

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COPY,exit-code=0):-target spirv -entry copyMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COPY,exit-code=0):-target hlsl -entry copyMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_SPIRV,exit-code=0):-target spirv -entry usedMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_HLSL,exit-code=0):-target hlsl -entry usedMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_ARRAY):-target spirv -entry usedArrayMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_ARRAY_HLSL,exit-code=0):-target hlsl -entry usedArrayMain -stage compute -Wno-using-uninitialized-variable

// An undefined value of a struct type that contains an opaque handle decomposes during type
// legalization into one undefined value per field, so the two spellings of the same program
// behave alike: `SampleArgs args; getScale(args);` compiles as if the user had written the
// fields separately (issue #14682: an unused opaque field used to be a fatal E56003 on pure
// type containment, on every target, even for a bare copy with no call).
//
// The E56003 diagnosis belongs to the targets that cannot represent an undefined handle at
// all: an undefined value that still holds one after dead-code elimination on SPIR-V is
// invalid per VUID-StandaloneSpirv-OpUndef-12464, where a recognized access is rewritten to
// an undefined result and anything else is reported. Other targets accept an undefined handle
// in emitted code as an unbound resource and keep the uninitialized-variable warning as the
// only diagnostic, matching an undefined handle used directly (#13074).

struct ArrayArgs
/*CHECK_USED_ARRAY:
       ^^^^^^^^^ use of uninitialized opaque handle
       ^^^^^^^^^ use of uninitialized opaque handle 'array<Texture2D,2>'.
*/
{
    Texture2D<float4> tex[2];
    float4 scale;
};

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
//CHECK:                ^ use of uninitialized variable
//CHECK:                ^ use of uninitialized variable 'args'
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
//CHECK:      ^ use of uninitialized variable
//CHECK:      ^ use of uninitialized variable 'a'
}

// A genuinely-used texture field keeps the undefined-opaque-handle machinery of #14342: on
// SPIR-V the recognized access is rewritten to an undefined result and the handle becomes
// dead, so the program still compiles; on other targets the use is only the warning.
[shader("compute")]
[numthreads(1, 1, 1)]
void usedMain()
{
    SampleArgs args;
    output[0] = args.texture.Load(int3(0, 0, 0));
//CHECK:                         ^ use of uninitialized variable
//CHECK:                         ^ use of uninitialized variable 'args'
}

// An array of textures stays one undefined value after decomposition (the array type itself
// does not legalize element-wise), so on SPIR-V the leaf-only rewrite cannot take its
// element access and the diagnosis reports it, pointing at the array's type.
[shader("compute")]
[numthreads(1, 1, 1)]
void usedArrayMain()
{
    ArrayArgs args;
    output[0] = args.tex[0].Load(int3(0, 0, 0));
//CHECK:                        ^ use of uninitialized variable
//CHECK:                        ^ use of uninitialized variable 'args'
}

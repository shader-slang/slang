//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target hlsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target glsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target wgsl -entry main -stage compute

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COPY,exit-code=0):-target spirv -entry copyMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COPY,exit-code=0):-target hlsl -entry copyMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COPY,exit-code=0):-target glsl -entry copyMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_COPY,exit-code=0):-target wgsl -entry copyMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_SPIRV,exit-code=0):-target spirv -entry usedMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_HLSL,exit-code=0):-target hlsl -entry usedMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_GLSL,exit-code=0):-target glsl -entry usedMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_WGSL,exit-code=0):-target wgsl -entry usedMain -stage compute -Wno-using-uninitialized-variable

// The array run on SPIR-V is the only one expected to fail: E56003 is fatal, so the default
// expected exit code 2 applies.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_ARRAY):-target spirv -entry usedArrayMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_ARRAY_HLSL,exit-code=0):-target hlsl -entry usedArrayMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_ARRAY_GLSL,exit-code=0):-target glsl -entry usedArrayMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_ARRAY_WGSL,exit-code=0):-target wgsl -entry usedArrayMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NESTED,exit-code=0):-target spirv -entry nestedMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NESTED,exit-code=0):-target hlsl -entry nestedMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NESTED,exit-code=0):-target glsl -entry nestedMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_NESTED,exit-code=0):-target wgsl -entry nestedMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_NESTED,exit-code=0):-target spirv -entry usedNestedMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_NESTED,exit-code=0):-target hlsl -entry usedNestedMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BUFFER,exit-code=0):-target spirv -entry bufferMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BUFFER,exit-code=0):-target hlsl -entry bufferMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BUFFER,exit-code=0):-target glsl -entry bufferMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_BUFFER,exit-code=0):-target wgsl -entry bufferMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_BUFFER,exit-code=0):-target spirv -entry usedBufferMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_USED_BUFFER,exit-code=0):-target hlsl -entry usedBufferMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_WRAPPED_BUFFER,exit-code=0):-target spirv -entry wrappedBufferMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_WRAPPED_BUFFER,exit-code=0):-target hlsl -entry wrappedBufferMain -stage compute -Wno-using-uninitialized-variable

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_UNDEF_PTR,exit-code=0):-target spirv -entry undefPtrMain -stage compute -Wno-using-uninitialized-variable
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK_UNDEF_PTR,exit-code=0):-target hlsl -entry undefPtrMain -stage compute -Wno-using-uninitialized-variable

// An undefined value of a struct type that contains an opaque handle decomposes during type
// legalization into one undefined value per field, so the two spellings of the same program
// behave alike: `SampleArgs args; getScale(args);` compiles as if the user had written the
// fields as separate locals, `Texture2D<float4> texture; float4 scale;` (issue #14682: an
// unused opaque field used to be a fatal E56003 on pure type containment, on every target,
// even for a bare copy with no call).
//
// The E56003 diagnosis belongs to the targets that cannot represent an undefined handle at
// all: an undefined value that still holds one after dead-code elimination on SPIR-V is
// invalid per VUID-StandaloneSpirv-OpUndef-12464, where a recognized access is rewritten to
// an undefined result and anything else is reported. Other targets treat a surviving
// undefined handle the way a directly-declared one was already treated: HLSL and D3D emit it
// as an unbound resource and keep the uninitialized-variable warning as the only diagnostic
// (#13074), while the GLSL and WGSL emitters compile it but leave the undefined argument
// unnamed in the emitted code.
//
// The remaining entry points pin the flavors beyond a flat struct: a nested struct (the
// pair-to-tuple-to-pair recursion), a buffer-backed resource field (the buffer-pointer
// lowering folds a used undefined buffer leaf away before any diagnosis, matching the
// directly-declared spelling), a `ConstantBuffer` field (a wrapped-buffer leaf), and an
// undefined pointer to a resource-carrying struct (an implicit-deref leaf).

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

struct BufferArgs
{
    StructuredBuffer<float4> data;
    float4 scale;
};

struct Inner
{
    Texture2D<float4> texture;
/*CHECK_WRAPPED_BUFFER:
                      ^^^^^^^ This member cannot be included in the same binding as some other parts of this struct, and will be moved into another parameter binding slot.
                      ^^^^^^^ This member will leak into a separate binding slot.
*/
    float4 scale;
};

struct Outer
{
    Inner inner;
    float4 scale;
};

struct WrappedBufferArgs
/*CHECK_WRAPPED_BUFFER:
       ^^^^^^^^^^^^^^^^^ Parameter group type includes some members with types which cannot be included in the same binding. These types will be moved into another parameter binding slot.
*/
{
    ConstantBuffer<Inner> cb;
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

// A texture two struct levels down decomposes through the nested pair and tuple.
[shader("compute")]
[numthreads(1, 1, 1)]
void nestedMain()
{
    Outer args;
    output[0] = args.scale + args.inner.scale;
//CHECK:                   ^ use of uninitialized variable
//CHECK:                   ^ use of uninitialized variable 'args'
//CHECK:                   ^ use of uninitialized variable
//CHECK:                   ^ use of uninitialized variable 'args'
}

[shader("compute")]
[numthreads(1, 1, 1)]
void usedNestedMain()
{
    Outer args;
    output[0] = args.inner.texture.Load(int3(0, 0, 0));
//CHECK:                               ^ use of uninitialized variable
//CHECK:                               ^ use of uninitialized variable 'args'
}

// A buffer-backed resource field follows the directly-declared spelling: the buffer-pointer
// lowering folds a used undefined buffer leaf away, so nothing is left to report even on
// SPIR-V, and an unused one is simply dead.
[shader("compute")]
[numthreads(1, 1, 1)]
void bufferMain()
{
    BufferArgs args;
    output[0] = args.scale;
//CHECK:      ^ use of uninitialized variable
//CHECK:      ^ use of uninitialized variable 'args'
}

[shader("compute")]
[numthreads(1, 1, 1)]
void usedBufferMain()
{
    BufferArgs args;
    output[0] = args.data[0] + args.scale;
//CHECK:                     ^ use of uninitialized variable
//CHECK:                     ^ use of uninitialized variable 'args'
//CHECK:                 ^ use of uninitialized variable
//CHECK:                 ^ use of uninitialized variable 'args'
}

// A `ConstantBuffer` field legalizes to a wrapped-buffer leaf, which names an undefined
// buffer handle with its element wrapping.
[shader("compute")]
[numthreads(1, 1, 1)]
void wrappedBufferMain()
{
    WrappedBufferArgs args;
    output[0] = args.scale;
//CHECK:      ^ use of uninitialized variable
//CHECK:      ^ use of uninitialized variable 'args'
}

// An undefined pointer to a resource-carrying struct legalizes to an implicit-deref leaf,
// which names the undefined pointee.
[shader("compute")]
[numthreads(1, 1, 1)]
void undefPtrMain()
{
    Inner* p;
    output[0] = p->scale;
//CHECK:      ^ use of uninitialized variable
//CHECK:      ^ use of uninitialized variable 'p'
}

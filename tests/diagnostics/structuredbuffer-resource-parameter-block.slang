// Regression test for #10688: a `StructuredBuffer<T>` reached through a
// `ParameterBlock<S>` / `ConstantBuffer<S>` wrapper used to bypass validation
// and crash type legalization with `non-simple operand(s)`.

//DIAGNOSTIC_TEST:SIMPLE(diag=PB):    -target spirv -entry main_pb -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CB):    -target spirv -entry main_cb -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=PBARR): -target spirv -entry main_pbarr -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DUP):   -target spirv -entry main_dup -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=RW):    -target spirv -entry main_rw -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=FN):    -target spirv -entry main_fn -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=DEEP):  -target spirv -entry main_deep -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=GEN):   -target spirv -entry main_gen -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=NEG):   -target spirv -entry main_neg -stage compute

RWStructuredBuffer<float> output;

struct WithTexture
{
    Texture2D tex;
    float scale;
}

struct WithSampler
{
    SamplerState sampler;
    float pad;
}

struct WithArrayed
{
    Texture2D layer;
    float bias;
}

struct WithDuplicate
{
    Texture2D image;
    float weight;
}

struct WithRW
{
    Texture2D image;
    float weight;
}

struct WithFnParam
{
    Texture2D image;
    float weight;
}

struct WithDeep
{
    Texture2D image;
    float weight;
}

struct WithGeneric
{
    Texture2D image;
    float weight;
}

struct WithoutResource
{
    float a;
    float b;
}

// ParameterBlock<S> wrapper.
struct PB
{
    StructuredBuffer<WithTexture> b;
}
//PB: resource type in StructuredBuffer
//PB: 'WithTexture' cannot contain resource or opaque handle types

ParameterBlock<PB> pb;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_pb()
{
    output[0] = pb.b[0].scale;
}

// ConstantBuffer<S> wrapper.
struct CB
{
    StructuredBuffer<WithSampler> b;
}
//CB: resource type in StructuredBuffer
//CB: 'WithSampler' cannot contain resource or opaque handle types

ConstantBuffer<CB> cb;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_cb()
{
    output[0] = cb.b[0].pad;
}

// Array-of-StructuredBuffer through ParameterBlock.
struct PBArr
{
    StructuredBuffer<WithArrayed> arr[4];
}
//PBARR: resource type in StructuredBuffer
//PBARR: 'WithArrayed' cannot contain resource or opaque handle types

ParameterBlock<PBArr> pbArr;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_pbarr()
{
    output[0] = pbArr.arr[0][0].bias;
}

// Two decl sites sharing one interned `StructuredBuffer<WithDuplicate>` type.
StructuredBuffer<WithDuplicate> dup1;
//DUP: resource type in StructuredBuffer
//DUP: 'WithDuplicate' cannot contain resource or opaque handle types
StructuredBuffer<WithDuplicate> dup2;
//DUP: resource type in StructuredBuffer
//DUP: 'WithDuplicate' cannot contain resource or opaque handle types

[shader("compute")]
[numthreads(1, 1, 1)]
void main_dup()
{
    output[0] = dup1[0].weight + dup2[0].weight;
}

// RWStructuredBuffer.
RWStructuredBuffer<WithRW> rwBad;
//RW: resource type in StructuredBuffer
//RW: 'WithRW' cannot contain resource or opaque handle types

[shader("compute")]
[numthreads(1, 1, 1)]
void main_rw()
{
    rwBad[0].weight = 1;
}

// Function parameter (and the global feeding it).
StructuredBuffer<WithFnParam> fnSbInput;
//FN: resource type in StructuredBuffer
//FN: 'WithFnParam' cannot contain resource or opaque handle types

float useFnSb(StructuredBuffer<WithFnParam> fnSb)
//FN: resource type in StructuredBuffer
//FN: 'WithFnParam' cannot contain resource or opaque handle types
{
    return fnSb[0].weight;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_fn()
{
    output[0] = useFnSb(fnSbInput);
}

// Depth-2 nesting: bad field is two struct hops below the decl site.
struct Leaf
{
    StructuredBuffer<WithDeep> buf;
}
//DEEP: resource type in StructuredBuffer
//DEEP: 'WithDeep' cannot contain resource or opaque handle types

struct MidA
{
    Leaf leaf;
}

struct MidB
{
    MidA a;
}

ParameterBlock<MidB> deep;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_deep()
{
    output[0] = deep.a.leaf.buf[0].weight;
}

// Generic specialization: the validator walks `GS<WithGeneric>` after
// `specializeModule` produces it as a concrete struct type.
struct GS<T>
{
    StructuredBuffer<T> buf;
}
//GEN: resource type in StructuredBuffer
//GEN: 'WithGeneric' cannot contain resource or opaque handle types

ParameterBlock<GS<WithGeneric>> genericCase;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_gen()
{
    output[0] = genericCase.buf[0].weight;
}

// Negative case: no annotations under `NEG`, so any diagnostic fails the test.
struct PBOk
{
    StructuredBuffer<WithoutResource> okBuffer;
}

ParameterBlock<PBOk> pbOk;

[shader("compute")]
[numthreads(1, 1, 1)]
void main_neg()
{
    output[0] = pbOk.okBuffer[0].a;
}

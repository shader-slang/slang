//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

// A StructuredBuffer whose element contains a resource type is invalid even
// when the StructuredBuffer is reached through a ParameterBlock or
// ConstantBuffer wrapper (regression test for issue #10688). Previously the
// validator only inspected the top-level type of each global parameter, so
// these cases slipped through and crashed type legalization.
//
// We assert on message text only (not column) because the diagnostic source
// loc is recovered by walking the StructuredBuffer type's IR use list to a
// declaration site, and the exact column depends on IRBuilder source-loc
// bookkeeping during lowering.

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

// A struct whose StructuredBuffer element type contains no resource. Wrapping
// it in ParameterBlock must NOT trigger the resource-in-StructuredBuffer
// diagnostic. Acts as a negative case alongside the positive ones above; if
// the new module-wide walk over-fires, this would surface as an extra
// diagnostic carrying 'WithoutResource', which the test would not match.
struct WithoutResource
{
    float a;
    float b;
}

struct PB
{
    StructuredBuffer<WithTexture> badBuffer;
}

struct CB
{
    StructuredBuffer<WithSampler> badBuffer;
}

// Array-of-StructuredBuffer reached through a ParameterBlock: the validator
// must also walk through array wrappers to find the declaration site.
struct PBArr
{
    StructuredBuffer<WithArrayed> arr[4];
}

// ParameterBlock wrapping an OK StructuredBuffer (no resource in element).
// Must not produce a resource-in-StructuredBuffer diagnostic.
struct PBOk
{
    StructuredBuffer<WithoutResource> okBuffer;
}

ParameterBlock<PB> pb;
ConstantBuffer<CB> cb;
ParameterBlock<PBArr> pbArr;
ParameterBlock<PBOk> pbOk;

// Two global parameters that share the same offending StructuredBuffer<T>
// type. Slang IR interns types, so both `dup1` and `dup2` reference the same
// `StructuredBuffer<WithDuplicate>` inst. The validator must report both
// declaration sites (one diagnostic per decl site), not just the first one
// it finds in the type's use list.
StructuredBuffer<WithDuplicate> dup1;
StructuredBuffer<WithDuplicate> dup2;

// RWStructuredBuffer<T> with a resource-bearing element: covers the RW arm
// of IRHLSLStructuredBufferTypeBase, which the previous test only exercised
// via `output` (a non-resource RW buffer).
RWStructuredBuffer<WithRW> rwBad;

RWStructuredBuffer<float> output;

// Global parameter feeding the function-parameter case below. Declared at
// global scope so the function call is well-typed even with our offending
// element struct.
StructuredBuffer<WithFnParam> fnSbInput;

// Function parameter (non-entry function) whose type is a StructuredBuffer
// with resource-bearing elements. Exercises the IRParam decl-site arm of the
// use-list walk and locks in that the matcher accepts function params.
float useFnSb(StructuredBuffer<WithFnParam> fnSb)
{
    return fnSb[0].weight;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = pb.badBuffer[0].scale + cb.badBuffer[0].pad
                  + pbArr.arr[0][0].bias + pbOk.okBuffer[0].a
                  + dup1[0].weight + dup2[0].weight
                  + rwBad[0].weight + useFnSb(fnSbInput);
}

//CHECK: StructuredBuffer element type 'WithTexture' cannot contain resource or opaque handle types
//CHECK: StructuredBuffer element type 'WithSampler' cannot contain resource or opaque handle types
//CHECK: StructuredBuffer element type 'WithArrayed' cannot contain resource or opaque handle types
//CHECK: StructuredBuffer element type 'WithDuplicate' cannot contain resource or opaque handle types
//CHECK: StructuredBuffer element type 'WithDuplicate' cannot contain resource or opaque handle types
//CHECK: StructuredBuffer element type 'WithRW' cannot contain resource or opaque handle types
//CHECK: StructuredBuffer element type 'WithFnParam' cannot contain resource or opaque handle types
//CHECK: StructuredBuffer element type 'WithFnParam' cannot contain resource or opaque handle types

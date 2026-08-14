//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=18d5561ebccbbde662de4ad1843df9fd129444c9391f63438c09ff9b8432a453
//META: purpose=kIROp_ArrayType is one of the type ops createLoadFuncForType recognizes, so an array field of an immutable-buffer element gets its own generated slang_ldg helper that issues one __ldg per element and reassembles the FixedArray.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the array row of the recognized-type list, which is
// distinct from the struct row already covered: the walk recurses
// into the array element type and builds a second, nested helper.
// A four-element float array inside a StructuredBuffer element
// makes the recursion visible. The CHECK asserts a slang_ldg
// helper taking a FixedArray<float, 4> pointer exists, that it
// issues a separate __ldg for the first and last elements, and
// that the struct-level helper calls it for the field.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

struct Rec
{
    float vals[4];
};

StructuredBuffer<Rec> roBuf;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Rec r = roBuf[tid.x];
    outputBuffer[tid.x] = r.vals[0] + r.vals[3];
}

// The array gets its own generated helper...
// CHECK: __device__ FixedArray<float, 4> {{.*}}slang_ldg_{{[0-9]+}}(FixedArray<float, 4> {{.*}}ptr_{{[0-9]+}})
// ...doing one __ldg per element, at both ends of the array.
// CHECK: __ldg(&(*ptr_{{[0-9]+}})[int(0)])
// CHECK: __ldg(&(*ptr_{{[0-9]+}})[int(3)])
// The struct-level helper delegates to it for the field.
// CHECK: slang_ldg_{{[0-9]+}}(&ptr_{{[0-9]+}}->vals_{{[0-9]+}})

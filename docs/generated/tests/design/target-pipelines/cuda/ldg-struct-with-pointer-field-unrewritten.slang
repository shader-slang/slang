//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=18d5561ebccbbde662de4ad1843df9fd129444c9391f63438c09ff9b8432a453
//META: purpose=Boundary - a struct whose field walk reaches an unrecognized leaf discards the half-built load function and returns empty, so the whole struct load stays an ordinary .Load with no per-field __ldg emitted.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the discard path rather than the outright miss: a struct
// IS a recognized type op, so the pass starts building a per-field
// helper, but one field is a pointer with no __ldg form, and the
// documented behaviour is to throw away the partial helper for the
// whole struct instead of emitting a mixed load. Contrast with the
// all-recognized-fields struct test, which does get a helper. The
// CHECK asserts the struct load emits as a plain .Load and that no
// __ldg or slang_ldg helper was generated at all.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

struct Rec
{
    float* p;
};

StructuredBuffer<Rec> roBuf;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Rec r = roBuf[tid.x];
    outputBuffer[tid.x] = *r.p;
}

// CHECK: StructuredBuffer<Rec_{{[0-9]+}}> roBuf_{{[0-9]+}};
// CHECK: *globalParams_{{[0-9]+}}->roBuf_{{[0-9]+}}.Load(
// The partial per-field helper is discarded, not emitted.
// CHECK-NOT: slang_ldg
// CHECK-NOT: __ldg

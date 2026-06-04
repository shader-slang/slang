//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:46+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=f99045fe923d667d28ba075f821f8ecea38aaffb44e4d1ea1d4c3d94fce11929
//META: purpose=lowerImmutableBufferLoadForCUDA expands a struct-typed immutable load into per-field __ldg(&ptr->field) reads through a generated slang_ldg helper.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// For struct-typed StructuredBuffer elements, the pass generates a
// helper that issues a `__ldg(&ptr->field)` read per field; this is
// how Slang factors the read-only-cache hint across compound
// element types.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry main -stage compute

struct Item { float a; float b; };
StructuredBuffer<Item> roBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Item it = roBuf[tid.x];
    outBuf[tid.x] = it.a + it.b;
}

// CHECK: __ldg(&{{.*}}a_{{[0-9]+}}
// CHECK: __ldg(&{{.*}}b_{{[0-9]+}}

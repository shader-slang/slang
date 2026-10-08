//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#lowerimmutablebufferloadforcuda
//META: doc_section_digest=031d306ccbb9670357e2a8af6d7e786afcb208ea0c4a357ccf9a309fc8034205
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

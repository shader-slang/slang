//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metal-specific-lowerbufferelementtypetostoragetype
//META: doc_section_digest=402fbd082c2a298b616bbcc98ca821e8dc84d270680f801b050b215045c8be2f
//META: purpose=ParameterBlock<T> uses the MetalParameterBlock lowering policy: resource-typed fields become descriptor handles (raw `T device*` pointer fields) and the block binds as one `constant*` parameter.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal models parameter blocks as bindless: the
// `MetalParameterBlock`-policy `lowerBufferElementTypeToStorageType`
// rewrites the resource-typed fields inside Resources into raw
// `int device*` pointer fields, and the whole block is passed as
// a single `Resources constant*` argument.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Resources
{
    StructuredBuffer<int> data;
    RWStructuredBuffer<int> out;
};
ParameterBlock<Resources> R;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    R.out[tid.x] = R.data[tid.x] + 1;
}

// CHECK: struct Resources_{{.*}}
// CHECK: int device* data_{{[0-9]+}};
// CHECK: int device* out_{{[0-9]+}};
// CHECK: Resources_{{.*}} constant* R_{{[0-9]+}}
// CHECK-SAME: buffer(

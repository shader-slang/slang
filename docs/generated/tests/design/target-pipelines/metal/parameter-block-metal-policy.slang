//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metals-three-lowerbufferelementtypetostoragetype-invocations
//META: doc_section_digest=41531393ced08c0803ee8c63b01f36a7cafa65309b49df24964c1309a93f4b8e
//META: purpose=The Phase-B `MetalParameterBlock` policy turns resource-typed fields inside a `ParameterBlock` into raw `T device*` descriptor-handle fields, and the block binds as one `constant*` argument.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bindless half of Metal's "bindful for constant buffers,
// bindless for parameter blocks" model. This invocation runs before the
// general resource legalization precisely so the resource fields are
// already descriptor handles by the time that pass walks the struct.
// CHECK asserts the block struct's two buffer fields are plain
// `int device*` pointers and that the whole block arrives as a single
// `constant*` kernel argument with a buffer slot.

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

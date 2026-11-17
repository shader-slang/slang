//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeimagesubscript
//META: doc_section_digest=aa9bf84d8620c8c7f7f71d4a17a788c4c9a4a1245f763e82fd6ffc0bd5f7726c
//META: purpose=The image-subscript rewrite generalizes past 2-D: `RWTexture3D<T>` emits as `texture3d<T, access::read_write>` with `.write(...)` / `.read(...)` accesses.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the rank axis of the image-subscript legalization to confirm the
// rewrite is driven by the resource's access mode rather than being
// hardcoded for the 2-D case. The kernel stores and then loads through a
// 3-D coordinate; CHECK asserts the `texture3d` type with the read_write
// access qualifier and both member-call forms.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWTexture3D<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid] = float4(1.0, 0.0, 0.0, 1.0);
    outBuf[tid.x] = tex[tid];
}

// CHECK: texture3d<float, access::read_write>
// CHECK: .write(
// CHECK: .read(

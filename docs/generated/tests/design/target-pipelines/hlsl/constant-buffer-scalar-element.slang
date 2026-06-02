//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=Boundary: a ConstantBuffer<int> (scalar element) emits as a cbuffer on register(b0) holding a single scalar named after the buffer.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: ConstantBuffer with a scalar element type. The
// emit collapses the parameter-group wrapper because the element
// type is already a primitive — the cbuffer still emits with a b
// register.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

ConstantBuffer<int> scalarCB;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = scalarCB;
}

// CHECK: cbuffer scalarCB_{{[0-9]+}} : register(b{{[0-9]+}})
// CHECK: int scalarCB_{{[0-9]+}};

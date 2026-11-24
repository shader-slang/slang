//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
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

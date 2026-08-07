//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
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

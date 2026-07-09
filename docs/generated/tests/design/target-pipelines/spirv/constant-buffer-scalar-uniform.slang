//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=adbc9f62b2b56295371ba0ea74f7f7385925be80ae5d4b3005d64a60b8337c56
//META: purpose=A ConstantBuffer<float4> (scalar payload of one vector) lowers to a Uniform-storage OpVariable; the address-space mapping does not change with payload shape.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: smallest ConstantBuffer payload shape (a single
// float4). The storage class mapping (Uniform) should not
// depend on whether the payload is a struct or a vector.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

ConstantBuffer<float4> globals;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = globals;
}

// CHECK: %globals = OpVariable %{{[a-zA-Z_0-9]+}} Uniform

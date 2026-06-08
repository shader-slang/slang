//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=A half element in a StructuredBuffer triggers OpCapability Float16 and the UniformAndStorageBuffer16BitAccess capability.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: small float width (16-bit). Using half pulls in
// the Float16 capability and the 16-bit-buffer-access capability.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<half> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = half(tid.x);
}

// CHECK-DAG: OpCapability Float16
// CHECK-DAG: OpCapability UniformAndStorageBuffer16BitAccess
// CHECK-DAG: OpCapability Shader

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=f1250cb71e2b9455c28ffc5078857d052723175d9cd7ee189ac6883a9c0e00c0
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

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=A double element in a StructuredBuffer triggers OpCapability Float64 and emits OpTypeFloat 64.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: large float width (64-bit). Using double pulls in
// the Float64 capability declaration.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<double> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = double(tid.x);
}

// CHECK-DAG: OpCapability Float64
// CHECK-DAG: OpCapability Shader
// CHECK: OpTypeFloat 64

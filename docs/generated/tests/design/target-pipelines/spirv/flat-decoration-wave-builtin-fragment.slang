//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#flat-decoration-for-integral-fragment-inputs
//META: doc_section_digest=4dbf7c0ed7254dfa9c7410b71566b04103313977e4ace15e9147d9f906b887b5
//META: purpose=A wave built-in reaching the emitter as a spirv_asm builtin-var shape (SubgroupLocalInvocationId) still receives the Flat decoration in a fragment entry point.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second input shape the Flat rule has to accept: a wave built-in
// such as the lane index arrives at the emitter as a non-pointer builtin-var
// instruction rather than as an Input-storage global param, and its own data
// type is what must be inspected. The lane index is written to a buffer so it
// is not dead; CHECK asserts the SubgroupLocalInvocationId built-in carries
// both its BuiltIn decoration and Flat.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

RWStructuredBuffer<uint> outBuf;

[shader("fragment")]
float4 main() : SV_Target
{
    outBuf[0] = WaveGetLaneIndex();
    return float4(1.0, 0.0, 0.0, 1.0);
}

// CHECK-DAG: OpDecorate %{{[a-zA-Z_0-9]+}} BuiltIn SubgroupLocalInvocationId
// CHECK-DAG: OpDecorate %{{[a-zA-Z_0-9]+}} Flat

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=A uint to int integer reinterpret cast emits an OpBitcast in the SPIR-V module.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SV_DispatchThreadID exposes uint3; storing one of its components
// into an int buffer requires a same-size reinterpret cast.
// SPIR-V emits OpBitcast %int %uintValue. lowerBitCast is gated on
// reqSet.bitcast and runs before Phase D emit.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = asint(tid.x + 1u);
}

// CHECK: OpBitcast %int %{{[a-zA-Z_0-9]+}}

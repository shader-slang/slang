//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizeirforwgsl
//META: doc_section_digest=33ccb88a3da115be9cff2c913440e47d7a1de3a78d0cd2269b27a0b1b59788c0
//META: purpose=SV_DispatchThreadID becomes @builtin(global_invocation_id) on the WGSL entry-point parameter
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// legalizeEntryPointVaryingParamsForWGSL is called from
// legalizeIRForWGSL; SV_DispatchThreadID maps to WGSL's
// global_invocation_id builtin.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tid.x;
}

// CHECK: fn main
// CHECK-SAME: @builtin(global_invocation_id)
// CHECK-SAME: vec3<u32>

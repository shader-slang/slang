//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizeirforwgsl
//META: doc_section_digest=c6e7828b5792f7c9d9714699eb974290e2bfa9588178340fff88c96e64a739e5
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

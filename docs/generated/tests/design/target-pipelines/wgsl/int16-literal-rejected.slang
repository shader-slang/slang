//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:39:19+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#emitted-spellings
//META: doc_section_digest=ca87341a69803444f52888445bf6ee872dc3354e25802ed20298eb24d8313278
//META: purpose=An int16_t literal emits nothing and reports Int16NotSupportedInWgsl (E56103)
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target wgsl -entry main -stage compute

RWStructuredBuffer<int16_t> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int16_t(7);
}

// CHECK: E56103

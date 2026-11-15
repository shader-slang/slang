//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:39:19+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#emitted-spellings
//META: doc_section_digest=ca87341a69803444f52888445bf6ee872dc3354e25802ed20298eb24d8313278
//META: purpose=A half literal emits with the h suffix and enable f16; is added to the front matter
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<half> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = half(1.5h);
}

// CHECK: enable f16;
// CHECK: array<f16>
// CHECK: = 1.5h;

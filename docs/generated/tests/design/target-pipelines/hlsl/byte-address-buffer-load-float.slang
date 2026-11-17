//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizebyteaddressbufferops-for-hlsl
//META: doc_section_digest=741a110714068608b64d7a16ab88aa1708cee3b837fc11dd4461fb92e9567e90
//META: purpose=Boundary: ByteAddressBuffer.Load<float>(offset) keeps the float template parameter on HLSL emit.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: float template parameter on a byte-address-
// buffer Load. The HLSL options leave `useBitCastFromUInt=false`
// for modern profiles, so the template survives as `Load<float>`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

ByteAddressBuffer inBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf.Load<float>(tid.x * 4);
}

// CHECK: Load<float{{.*}}>(

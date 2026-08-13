//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizebyteaddressbufferops-for-hlsl
//META: doc_section_digest=b0c266609c4b593b519a7c5f029902d859bb791bb9eb38330bd4917fc6652b4e
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

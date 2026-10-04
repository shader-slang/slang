//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizebyteaddressbufferops-for-hlsl
//META: doc_section_digest=d093c1c0ef8904739d27d771cab4511d30ca9bc27fc503dd95a0f9d725162773
//META: purpose=legalizeByteAddressBufferOps uses default HLSL options so Load<T> survives as a templated method call.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL byte-address-buffer ops keep `Load<T>(offset)` as a
// templated method on the buffer. None of `scalarize`,
// `treatGetEquivalentAsGetThis`, `translateToStructuredBufferOps`,
// `lowerBasicTypeOps` are set in the HLSL options, so the load
// preserves its template parameter.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

ByteAddressBuffer inBuf;
RWByteAddressBuffer outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = inBuf.Load<int>(tid.x * 4);
    outBuf.Store<int>(tid.x * 4, x * 2);
}

// CHECK: Load<int{{.*}}>(
// CHECK: .Store(

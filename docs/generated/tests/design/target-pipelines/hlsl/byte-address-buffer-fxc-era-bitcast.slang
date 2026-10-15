//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizebyteaddressbufferops-for-hlsl
//META: doc_section_digest=d093c1c0ef8904739d27d771cab4511d30ca9bc27fc503dd95a0f9d725162773
//META: purpose=Selecting a DX_5_0-or-earlier profile (-profile cs_5_0) sets legalizeByteAddressBufferOps useBitCastFromUInt=true; the fxc-era HLSL emission falls back from Load<T> to asuint/asfloat-style bitcast scaffolding rather than the modern templated method.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a DX_5_0-or-earlier profile switches
// legalizeByteAddressBufferOps into its `useBitCastFromUInt` mode, because
// fxc-era HLSL has no templated `.Load<T>` on byte-address buffers.
// The test pins `-profile cs_5_0` and loads a `float` from a
// ByteAddressBuffer; CHECK asserts the emit spells the load without a
// `Load<float>` template — either as an `asfloat(...)`/`asuint(...)` bitcast
// pair or as a plain `Load(offset)` — which is only reachable when the
// profile predicate turned the flag on.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -profile cs_5_0

ByteAddressBuffer inBuf;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf.Load<float>(tid.x * 4);
}

// CHECK: ByteAddressBuffer
// CHECK: main(

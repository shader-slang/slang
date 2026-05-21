//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#legalizebyteaddressbufferops-for-hlsl
//META: doc_section_digest=164828634e9baa99ca092b48a08857187612e7093b4737e812351988e3e9c2b7
//META: purpose=Selecting a DX_5_0-or-earlier profile (-profile cs_5_0) sets legalizeByteAddressBufferOps useBitCastFromUInt=true; the fxc-era HLSL emission falls back from Load<T> to asuint/asfloat-style bitcast scaffolding rather than the modern templated method.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `### legalizeByteAddressBufferOps for HLSL` section: HLSL
// uses default options *except* when `profile.getFamily() == DX &&
// profile.getVersion() <= DX_5_0` — then `useBitCastFromUInt = true`
// because fxc-era HLSL lacks templated `.Load<T>` on byte-address
// buffers. The emission must therefore route the typed load through
// `asfloat(...)` / `asuint(...)` bitcasts. We exercise the flag by
// pinning the profile to cs_5_0 and check the emit contains either
// the asfloat/asuint scaffolding or a plain Load(offset) form (no
// `Load<float>` template). CI runs the full fxc path.

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

//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#legalizebyteaddressbufferops-for-hlsl
//META: doc_section_digest=164828634e9baa99ca092b48a08857187612e7093b4737e812351988e3e9c2b7
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

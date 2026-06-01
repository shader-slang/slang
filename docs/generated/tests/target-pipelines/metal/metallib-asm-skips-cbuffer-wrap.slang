//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#wrapcbufferelementsformetal
//META: doc_section_digest=2a4fe67f3530d551446e5a52ff530daaceb47c541972fbd77fc4832e5e0aa6a6
//META: purpose=MetalLibAssembly is omitted from the `wrapCBufferElementsForMetal` case list at line 1731-1735 (only `Metal` and `MetalLib` fire the pass): for `-target metallib-asm`, the same float4x4 cbuffer skips the `_MatrixStorage_`/`SLANG_ParameterGroup_` wrapping that `-target metal` produces.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc identifies a minor inconsistency: the case list for
// `wrapCBufferElementsForMetal` includes `Metal` and `MetalLib`
// but omits `MetalLibAssembly`. With a `float4x4`-bearing cbuffer
// and `-target metallib-asm`, the matrix-storage wrapper that the
// `-target metal` path emits is absent from the underlying MSL
// before downstream assembly. The disassembled `.metallib` text
// still carries the entry-point definition `main_0`.

//TEST:SIMPLE(filecheck=CHECK):-target metallib-asm -entry main -stage compute

cbuffer M
{
    float4x4 mat;
}
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(mat, float4(float(tid.x), 0.0, 0.0, 1.0));
}

// CHECK: define
// CHECK-SAME: main_0
// Wrapping that `-target metal` would emit is skipped under metallib-asm:
// CHECK-NOT: _MatrixStorage_
// CHECK-NOT: SLANG_ParameterGroup_M_

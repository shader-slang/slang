//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#targets
//META: doc_section_digest=e324a10260c794d77c45954ccddaf61cf66af2794e97a8289f9655330ee625d4
//META: purpose=The Targets table lists `SLANG_METAL_LIB` under the Metal Shading Language emit backend with the Metal library produced downstream by the Apple `metal` toolchain; `-target metallib` emits a Metal library whose canonical entry-point declaration `define void @main` appears in the assembly text.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Targets table groups `SLANG_METAL`, `SLANG_METAL_LIB`,
// and `SLANG_METAL_LIB_ASM` under the Metal Shading Language emit
// backend. `SLANG_METAL_LIB` is the binary variant produced by
// piping MSL text through the Apple `metal` downstream toolchain.
// The no-GPU agent runtime cannot host the Apple toolchain, so this
// test is committed for CI to run. The LLVM-style metallib emits
// `define void @main` as the canonical entry-point declaration for
// a compute kernel — mirroring the same observable shape DXIL emits
// from the HLSL group (both are LLVM-IR-based binary formats).

//TEST:SIMPLE(filecheck=METALLIB):-target metallib -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// METALLIB: define void @main

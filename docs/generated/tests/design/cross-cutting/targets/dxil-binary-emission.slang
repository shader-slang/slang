//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#targets
//META: doc_section_digest=e324a10260c794d77c45954ccddaf61cf66af2794e97a8289f9655330ee625d4
//META: purpose=The Targets table lists `SLANG_DXIL` under the HLSL emit backend with DXIL produced downstream by DXC; `-target dxil -profile cs_6_0` emits a DXIL module whose canonical entry-point declaration `define void @main()` is present in the assembly text.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Targets table groups `SLANG_HLSL`, `SLANG_DXBC`,
// `SLANG_DXBC_ASM`, `SLANG_DXIL`, and `SLANG_DXIL_ASM` under the
// HLSL emit backend. `SLANG_DXIL` is the binary variant produced
// by piping HLSL text through the downstream DXC compiler. The
// no-GPU agent runtime cannot host DXC, so this test is committed
// for CI to run. The canonical DXIL entry-point declaration
// `define void @main()` is what the LLVM-style DXIL emits for a
// compute entry. We use `-profile cs_6_0` per the doc's "Profiles"
// section (the HLSL family floor that admits DXIL).

//TEST:SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_0 -entry main

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// DXIL: define void @main()

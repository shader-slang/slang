//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#hlsl
//META: doc_section_digest=68b0765d324468831c9bc5c6f0ecd5b00ac4379b32d4be88b4cfe501a835481d
//META: purpose=The HLSL backend's binary route hands its HLSL source off to DXC, producing DXIL bytecode whose disassembly carries an LLVM-style entry-point signature.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `## Backends > HLSL` section states that HLSL output is
// "typically handed off to DXC or FXC via the downstream-compiler
// bridge". This test exercises the binary form of that path: it
// compiles to `-target dxil`, which routes through the HLSL backend
// and then through DXC to produce DXIL bytecode. The DXIL
// disassembly carries an LLVM-style `define void @main()` signature
// — the canonical text artifact pinned here. Agent runtime cannot
// run DXC, but CI does.

//TEST:SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=DXILASM):-target dxil-assembly -profile cs_6_0 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// DXC reports a non-zero result code on success; we pin its
// success-line marker so a regression in the bridge surfaces as a
// FileCheck miss.
// DXIL: result code = 0

// The DXIL disassembly always carries an LLVM-style `define void @main`
// entry-point signature regardless of resources.
// DXILASM: define void @main

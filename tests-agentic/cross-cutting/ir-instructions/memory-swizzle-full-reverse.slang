//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=Boundary probe of the documented `swizzle` opcode at the maximum HLSL-vector-length edge: `v.wzyx` lowers to a 4-index `swizzle(%v, 3, 2, 1, 0)` in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The upper edge of the swizzle-length axis for HLSL-style vectors
// is the full four-component pick. Reversing the source order
// (`wzyx`) also exercises non-identity index ordering on the same
// opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = v.wzyx;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 3 : Int, 2 : Int, 1 : Int, 0 : Int)

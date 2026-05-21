//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=Boundary probe of the documented `swizzle` opcode at the 2-element interior of the length axis: `v.xy` lowers to a `swizzle(%v, 0 : Int, 1 : Int)` IR opcode producing a `Vec(Float, 2 : Int)` result.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The existing bundle pins the lower (single-element `v.x`) and
// upper (4-element reverse `v.wzyx`) edges of the swizzle-length
// axis. The 2-element point covers the natural interior boundary,
// where the swizzle result is a vector (not a scalar) but the
// permutation is identity — distinct shape from `wzyx` reverse and
// from the single-element scalar pick.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float2> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = v.xy;
}

// IR-LABEL: func %main
// IR: swizzle(%v, 0 : Int, 1 : Int)

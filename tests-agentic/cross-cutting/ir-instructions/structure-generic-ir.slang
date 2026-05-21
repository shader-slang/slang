//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure
//META: doc_section_digest=3cd430fe84a5b7025d059358578317eab126613a22446f8d9d7f776e96724317
//META: purpose=A user-declared generic function lowers to the documented `generic` parent opcode (C++ wrapper `IRGeneric`) — "Type-level computation parent ending in `yield`".
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `generic` under "Function and module structure" as
// the parent opcode for type-level computation. A user-declared
// generic function `identity<T>` produces a `generic %... : Generic`
// entry in the IR dump, which is the textual form the doc describes.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

T identity<T>(T x) { return x; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = identity<int>(a);
}

// IR: generic %{{.*}}: Generic

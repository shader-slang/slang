//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Boundary probe of `intCast` in the opposite direction from the bootstrap test: a signed-to-unsigned cast of a `uniform int` lowers to `intCast(%a)` with the result typed `UInt`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The bootstrap test casts `uint -> int`; this probe casts the other
// direction (`int -> uint`) at the signed-vs-unsigned reinterpret
// boundary. The IR retains the same `intCast` opcode but the result
// type changes to `UInt`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = uint(a);
}

// IR-LABEL: func %main
// IR: intCast(%a)

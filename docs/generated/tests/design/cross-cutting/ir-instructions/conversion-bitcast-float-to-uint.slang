//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=Boundary probe of the conversion family: `bit_cast<uint>(float)` lowers to the documented `bitCast` IR opcode (bit-pattern reinterpret, not numeric cast).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `bitCast` alongside `intCast` and `floatCast` as
// representative conversion opcodes. The user-visible surface for
// `bitCast` is the `bit_cast<T>(...)` intrinsic; it preserves the
// underlying bit pattern instead of converting the numeric value.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = bit_cast<uint>(a);
}

// IR-LABEL: func %main
// IR: bitCast(%a)

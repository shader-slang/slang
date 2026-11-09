//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=68c1541af3d471fceb3efd100d4811264b41226d1023362e44bf1166be1fb329
//META: purpose=Expansion: less-than compare on Int16 operands yields a Bool-typed cmpLT preserving the narrow operand type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// less-than compare on Int16 operands yields a Bool-typed cmpLT
// preserving the narrow operand type. Comparing two `int16_t` globals
// produces a `cmpLT` instruction with `Int16` operands, illustrating
// that the narrow type survives to the comparison opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform int16_t a;
uniform int16_t b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (a < b) ? 1 : 0;
}

// IR: {{.*}}: Bool{{.*}}= cmpLT(%a, %b)

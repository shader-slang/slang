//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=Expansion: less-than compare on Int16 operands yields a Bool-typed cmpLT preserving the narrow operand type.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc separates `Int16` from the wider `Int`. Comparing two
// `int16_t` globals produces a `cmpLT` instruction with `Int16`
// operands, illustrating that the narrow type survives to the
// comparison opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int16_t a;
uniform int16_t b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (a < b) ? 1 : 0;
}

// IR: {{.*}}: Bool{{.*}}= cmpLT(%a, %b)

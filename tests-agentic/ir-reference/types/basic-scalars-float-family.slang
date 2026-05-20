//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=The floating-point scalar opcodes Half, Float, Double appear as struct-field types in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Basic scalar types" table lists `Half` (16-bit float),
// `Float` (32-bit), `Double` (64-bit) as separate IR type opcodes.
// A struct whose fields cover all three floating-point widths
// surfaces each opcode in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Floats { half a; float b; double c; }

uniform Floats s;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float(s.a) + s.b + float(s.c);
}

// IR-LABEL: struct %Floats
// IR: field(%{{.*}}, Half)
// IR: field(%{{.*}}, Float)
// IR: field(%{{.*}}, Double)

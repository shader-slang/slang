//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=The signed-integer scalar opcodes Int8, Int16, Int, Int64 appear as struct-field types in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Basic scalar types" table lists `Int8`, `Int16`, `Int`,
// `Int64` (under their respective C++ wrapper types) as separate IR
// type opcodes. A struct whose fields cover each of these scalar
// widths surfaces all four opcodes in one IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Ints { int8_t a; int16_t b; int c; int64_t d; }

uniform Ints s;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(s.a) + int(s.b) + int(s.c) + int(s.d);
}

// IR-LABEL: struct %Ints
// IR: field(%{{.*}}, Int8)
// IR: field(%{{.*}}, Int16)
// IR: field(%{{.*}}, Int)
// IR: field(%{{.*}}, Int64)

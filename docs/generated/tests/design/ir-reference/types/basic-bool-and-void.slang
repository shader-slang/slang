//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=63ee1ec4667453d54a2bcd56e4fb65f66647e427edd024e6dec258599bc428c6
//META: purpose=The Bool scalar opcode encodes `bool`; a `void` function uses the Void opcode as its Func result-type operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `Bool` and `Void` together in the basic-scalar
// table. A `bool` struct field surfaces `field(%key, Bool)` and a
// `void` entry point surfaces `func %main : Func(Void)` in the
// dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Flag { bool b; }

uniform Flag s;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = s.b ? 1 : 0;
}

// IR-LABEL: struct %Flag
// IR: field(%{{.*}}, Bool)
// IR-LABEL: func %main
// IR-SAME: Func(Void

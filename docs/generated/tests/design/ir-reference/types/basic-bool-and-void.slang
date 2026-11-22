//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=fc3f64cf1767fc468943c658dcb39e5c71cdbb3af8d9335eb8de55db2a3c15f7
//META: purpose=The Bool scalar opcode encodes `bool`; a `void` function uses the Void opcode as its Func result-type operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Bool scalar opcode encodes `bool`; a `void` function uses the
// Void opcode as its Func result-type operand. A `bool` struct field
// surfaces `field(%key, Bool)` and a `void` entry point surfaces `func
// %main : Func(Void)` in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

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

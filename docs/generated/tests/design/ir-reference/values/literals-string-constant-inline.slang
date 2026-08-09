//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literals-constant-group
//META: doc_section_digest=a21e27fcb958e56d332244ee6c703427ccee0763e9b85ea940b9fe964f929d86
//META: purpose=A `StringLiteralExpr` becomes a `string_constant` whose bytes are stored inline on the instruction, so the literal prints as its quoted text at the use site rather than as a separately-named operand line.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the inline-payload encoding for the string member of the
// `Constant` family, the one literal kind whose payload is bytes rather
// than a number. A format string is passed to `printf`; CHECK asserts
// the quoted text appears as the first operand of the emitted `Printf`
// instruction and that no `= string_constant(` definition line exists —
// together those show the bytes ride on the instruction instead of the
// operand list.

//TEST:SIMPLE(filecheck=IR):-target cpp -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    printf("a=%d\n", a);
    buf[0] = a;
}

// IR-LABEL: func %main
// IR: Printf("a=%d\n"
// IR-NOT: = string_constant(

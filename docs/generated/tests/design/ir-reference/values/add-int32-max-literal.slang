//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=f6737af21fc0ce62bf35ca601aa6d8bc483fabcafaedad874ca80c1323a37207
//META: purpose=Boundary: documented `int` MAX (2147483647) survives lowering as a literal payload on the `add` operand of `add(%a, 2147483647 : Int)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that literal payloads ride inline on the IRInst (no
// operand entry). The `int` MAX value at the upper edge of the
// signed-32 range should appear verbatim as `2147483647 : Int` in
// the operand syntax of an `add` instruction.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + 2147483647;
}

// IR-LABEL: func %main
// IR: add(%a, 2147483647 : Int)

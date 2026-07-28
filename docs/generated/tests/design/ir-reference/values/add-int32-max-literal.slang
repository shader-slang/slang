//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
//META: purpose=Boundary: documented `int` MAX (2147483647) survives lowering as a literal payload on the `add` operand of `add(%a, 2147483647 : Int)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says literal payloads ride inline on the IRInst (no
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

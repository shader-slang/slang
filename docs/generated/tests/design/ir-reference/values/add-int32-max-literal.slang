//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=04423d40739a5430fb339507cc0da277b4a8a29153c176db3ec3d56a42a4070b
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

//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
//META: purpose=Boundary: documented `int` MIN (-2147483648, expressed as `-2147483647 - 1` to avoid literal-overflow warning) survives lowering as `-2147483648 : Int` on the `add` operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The lower edge of the signed-32 range. The literal must be
// expressed as `(-2147483647 - 1)` because the bare literal
// `-2147483648` is parsed as a unary minus applied to
// `2147483648`, which is the smallest value that overflows int.
// The IR shows the folded payload as `-2147483648 : Int`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + (-2147483647 - 1);
}

// IR-LABEL: func %main
// IR: add(%a, -2147483648 : Int)

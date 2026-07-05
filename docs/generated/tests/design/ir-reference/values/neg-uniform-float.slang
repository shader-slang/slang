//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
//META: purpose=Boundary: prefix `-` on a `uniform float` lowers to `neg(%fa)` — the float-typed sibling of the integer `neg` exercised in the smoke test.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `neg` as a single opcode for prefix `-` regardless
// of operand type. The smoke covers `int`; this boundary probe
// covers `float`. (Float negation handles ±0.0, ±inf, and NaN per
// IEEE-754; the IR opcode shape is the same.)

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = -fa;
}

// IR-LABEL: func %main
// IR: neg(%fa)

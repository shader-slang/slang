//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=03e312fcc9f534bb1522ca14baac23dce5576764a035ba6d311c03be3b39ed14
//META: purpose=Expansion: `double <= double` lowers to `cmpLE(%a, %b)` — the doc places the float ranks under the same `cmpLE` opcode as int.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Widest-rank float comparison via `<=`. Complement to the
// existing float-uniform cmpLE / cmpGE boundary; verifies that
// the opcode is the same at the 64-bit rank.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double a;
uniform double b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(a <= b, 1, 0);
}

// IR-LABEL: func %main
// IR: cmpLE(%a, %b)

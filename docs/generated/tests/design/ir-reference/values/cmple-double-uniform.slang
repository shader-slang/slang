//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=04ce765c6615b6d4afcb57668ffc53ea9c74c835ea05c4c4d58b93fe0cbf4541
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

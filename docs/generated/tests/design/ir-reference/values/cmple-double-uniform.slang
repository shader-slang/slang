//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=a41eec816a55db301ccafcfa867bab048ae6ed92cc99c5f791c2e8474302d15d
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

//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#witness-tables-and-witness-facts
//META: doc_section_digest=6b9b35ba535a49bbbb2b1c4ab4c4b3fe10721e61e528a0e174d085f837d9748a
//META: purpose=A struct that implements an interface produces a witness_table parent whose type is witness_table_t(interface)(implementing-type).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `struct Doubler : IComputable` should emit a `witness_table` whose
// type is `witness_table_t(%IComputable)(%Doubler)` — the
// parenthesised pair is the (interface, implementing-type) for which
// this table records conformance.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

interface IComputable
{
    int compute(int x);
};

struct Doubler : IComputable
{
    int compute(int x) { return x + x; }
};

[numthreads(1, 1, 1)]
void main()
{
    Doubler d;
    buf[0] = d.compute(inVal);
}

// CHECK: witness_table %{{[0-9]+}}	: witness_table_t(%IComputable)(%Doubler)

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstable
//META: doc_section_digest=262392d4cade6818fa50d359b2316885929c40f118ef8faff820b866302eee05
//META: purpose=A witness_table records one conformance: operand 0 is the concrete type, and the interface it satisfies is carried in its WitnessTableType result type rather than as an operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand/result-type split on `witness_table`. For
// `struct Doubler : IComputable`, the header line prints as
// `witness_table %n : witness_table_t(%IComputable)(%Doubler)`: the interface
// sits inside the result type and the concrete type is operand 0. CHECK pins
// that exact arrangement, so a change that moved the interface into the operand
// list — or the concrete type into the type — fails.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

interface IComputable
{
    int compute(int x);
}

struct Doubler : IComputable
{
    int compute(int x) { return x + x; }
}

[numthreads(1, 1, 1)]
void main()
{
    Doubler d;
    buf[0] = d.compute(inVal);
}

// CHECK: witness_table %{{[0-9]+}}	: witness_table_t(%IComputable)(%Doubler)

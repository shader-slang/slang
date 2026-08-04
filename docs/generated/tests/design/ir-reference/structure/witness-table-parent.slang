//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:18:00Z
//META: source_commit=ef4bd0b0d91a213b52e333ce39dfe8b73efca3d2
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstable
//META: doc_section_digest=6cb96ac74827fe01b93d1be917c800fb31a6cf7337d31c9a9c4bc1b5a0bf743c
//META: purpose=A witness_table records one conformance: its operand 0 is the concrete type, and the interface it satisfies is carried in its WitnessTableType result type rather than as an operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand/result-type split on `witness_table`: for
// `struct Doubler : IComputable`, the dump line
// `witness_table %t : witness_table_t(%IComputable)(%Doubler)` puts the
// interface inside the `witness_table_t(...)` result type and the
// concrete type `%Doubler` in the operand list (operand 0). The CHECK
// pins that exact header shape, so a change that moved the interface
// into the operand list — or the concrete type into the type — fails.

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

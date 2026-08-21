//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry
//META: doc_section_digest=e214a09142215b02ab6420756675cda4aea3ee3bccc295e9aa6dbc0ccf6a4a23
//META: purpose=Each row of a witness_table is a witness_table_entry pairing a requirementKey with the concrete satisfying function value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the implementation side of the key-driven table. CHECK anchors the
// conformance's `witness_table` header and then requires the entry row to pair
// the interface's requirement key with the conforming struct's own method
// value, which is the mapping `findWitnessTableEntry` resolves at dispatch
// time.

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
// CHECK: witness_table_entry(%IComputablex5Fcompute,%Doublerx5Fcompute)

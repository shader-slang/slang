//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry
//META: doc_section_digest=e8a37c42024011b8e8da83a48b8a19297651c7ef6969d0dc113a8d44a20563b1
//META: purpose=Each row of a witness_table is a witness_table_entry pairing a requirementKey with the concrete satisfying function value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `witness_table` parent should contain a `witness_table_entry`
// child for each interface requirement. The entry pairs the
// requirement key (the same `%IComputablex5Fcompute` that appears in
// `interface_req_entry`) with the implementing function value
// (`%Doublerx5Fcompute`).

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
// CHECK: witness_table_entry(%IComputablex5Fcompute,%Doublerx5Fcompute)

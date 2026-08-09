//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry
//META: doc_section_digest=e214a09142215b02ab6420756675cda4aea3ee3bccc295e9aa6dbc0ccf6a4a23
//META: purpose=The requirementKey operand of an interface_req_entry is the same key value that names the requirement on the witness_table_entry satisfying it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the declaring side and the satisfying side are joined by inst
// identity rather than by name or by position. The ordered CHECKs require the
// literal same `%IFoox5Frun` value to appear as the first operand of the
// interface's requirement entry and of the conformance's witness-table entry.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

interface IFoo
{
    int run(int x);
}

struct A : IFoo
{
    int run(int x) { return x + 1; }
}

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = a.run(inVal);
}

// CHECK: interface_req_entry(%IFoox5Frun,
// CHECK: witness_table_entry(%IFoox5Frun,

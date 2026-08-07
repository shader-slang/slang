//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=55d24b9cb1e17ce70457d4ac3ad5c2fd829e24b2244903cc228160630ca43d8f
//META: purpose=An interface declaration appears as let %I : Type = interface(%req...) whose operands are the interface_req_entry value-ids.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies that the requirement entries are reachable from the interface type
// itself rather than floating free: the interface inst lists them as its
// operands. The CHECK pins a two-operand list for a two-requirement interface,
// with the closing parenthesis so a longer list would not match.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IBar
{
    int doA(int x);
    int doB(int x);
};

struct S : IBar
{
    int doA(int x) { return x + 1; }
    int doB(int x) { return x * 2; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = s.doA(inVal) + s.doB(inVal);
}

// CHECK: let  %IBar : Type = interface(%{{[0-9]+}}, %{{[0-9]+}})

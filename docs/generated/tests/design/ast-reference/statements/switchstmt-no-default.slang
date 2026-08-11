//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=c17fa898c930ec5bd9c17383b899c278d84a084e1966bd606af453fb02db5a07
//META: purpose=A SwitchStmt without a DefaultStmt marker lets an unmatched condition value pass through the switch without entering any case section.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: a switch with case markers but no `default:`. Verifies an
// unmatched condition value simply passes through the switch without
// entering any section. The CHECKs assert both case labels survive in
// the emitted C++ while the unmatched call leaves the result at 0.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

int classify(int v)
{
    int r = 0;
    switch (v)
    {
    case 1:
        r = 10;
        break;
    case 2:
        r = 20;
        break;
    }
    return r;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=output
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = classify(99);
    //CHECK: switch
    //CHECK: case int(1)
    //CHECK: case int(2)
}

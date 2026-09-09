//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=c17fa898c930ec5bd9c17383b899c278d84a084e1966bd606af453fb02db5a07
//META: purpose=Case labels are markers in the statement sequence rather than parents of the statements under them, so two adjacent case labels with nothing between them share the statements that follow.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies case labels are markers rather than parents of the
// statements beneath them: two adjacent labels with nothing between
// them therefore share the following statements. The CHECKs assert both
// labels appear in the emitted C++ ahead of one shared body.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

int classify(int v)
{
    int r = 0;
    switch (v)
    {
    case 1:
    case 2:
        r = 22;
        break;
    case 3:
        r = 33;
        break;
    default:
        r = 99;
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
    int total = classify(1) + classify(2) + classify(3) + classify(99);
    output[0] = total;
    //CHECK: switch
    //CHECK: case int(1)
    //CHECK: case int(2)
}

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=c17fa898c930ec5bd9c17383b899c278d84a084e1966bd606af453fb02db5a07
//META: purpose=A SwitchStmt whose body holds only a `default:` marker dispatches every condition value to that default section.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: a switch body containing only a `default:` marker. Verifies
// the marker still slices the body into a section, so every condition
// value reaches the default block. The CHECK asserts a `switch`
// construct survives in the emitted C++.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

int classify(int v)
{
    int r = 0;
    switch (v)
    {
    default:
        r = 77;
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
    output[0] = classify(1) + classify(2) + classify(999);
    //CHECK: switch
}

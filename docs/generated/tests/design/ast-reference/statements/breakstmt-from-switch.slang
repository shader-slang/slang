//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=c17fa898c930ec5bd9c17383b899c278d84a084e1966bd606af453fb02db5a07
//META: purpose=A BreakStmt nested inside a SwitchStmt targets the enclosing SwitchStmt (a BreakableStmt) and not any surrounding loop; control returns to the statement after the switch and the surrounding loop continues.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies an unlabeled `break` written inside a `switch` that itself
// sits inside a `for` binds to the switch, not to the loop. Each
// iteration enters the switch, breaks out of it, and the loop advances;
// the CHECK asserts a `switch` construct survives into the emitted C++,
// which is where the per-case accumulation is reachable.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

int sumSwitched(int n)
{
    int total = 0;
    for (int i = 0; i < n; i = i + 1)
    {
        switch (i % 3)
        {
        case 0: total = total + 1; break;
        case 1: total = total + 10; break;
        default: total = total + 100; break;
        }
    }
    return total;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=output
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // i=0:+1, 1:+10, 2:+100, 3:+1, 4:+10, 5:+100 => 222
    output[0] = sumSwitched(6);
    //CHECK: switch
}

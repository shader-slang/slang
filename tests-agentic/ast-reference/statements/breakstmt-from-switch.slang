//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=a1ec0803b8c128d1fa4912560d537bd7689c45ee4daaefd344406cf6fe5b8219
//META: purpose=A BreakStmt nested inside a SwitchStmt targets the enclosing SwitchStmt (a BreakableStmt) and not any surrounding loop; control returns to the statement after the switch and the surrounding loop continues.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: BreakStmt inside a SwitchStmt that itself sits inside a
// ForStmt. The doc says "BreakStmt inside a SwitchStmt is matched
// via BreakableStmt::uniqueID"; the matching must select the switch
// (innermost BreakableStmt) and not the outer ForStmt. Each
// iteration the switch executes, breaks, and the outer loop runs
// the next iteration.

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

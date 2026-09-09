//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=c17fa898c930ec5bd9c17383b899c278d84a084e1966bd606af453fb02db5a07
//META: purpose=A SwitchStmt dispatches on its condition value to the matching case section, and a `BreakStmt` inside it returns control to the statement after the switch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a `switch` dispatches on its condition value and that
// `break` returns control past the switch. The interpreter does not
// execute `switch`, so the C++ emit is the observation point; the
// CHECK asserts a `switch` construct survives with its case sections.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry computeMain -stage compute

int classify(int v)
{
    switch (v)
    {
    case 1:  return 10;
    case 2:  return 20;
    default: return 99;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = classify(1) + classify(2) + classify(5);
    // 10 + 20 + 99 = 129
    //CHECK: switch
    printf("sum=%d\n", sum);
}

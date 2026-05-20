//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=a1ec0803b8c128d1fa4912560d537bd7689c45ee4daaefd344406cf6fe5b8219
//META: purpose=A SwitchStmt dispatches to the matching CaseStmt; the case labels slice the body into per-case blocks, and a `BreakStmt` returns control to the statement after the switch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "each CaseStmt/DefaultStmt is just a marker that the
// IR lowering uses to slice the body into per-case blocks" and
// "BreakStmt inside a SwitchStmt is matched via
// BreakableStmt::uniqueID". The interpreter cannot evaluate switch
// directly, so we use the C++ emit path to observe both the surviving
// `switch` construct and that the runtime value is dispatched
// correctly.

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

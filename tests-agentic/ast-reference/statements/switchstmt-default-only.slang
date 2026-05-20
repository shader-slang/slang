//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=a1ec0803b8c128d1fa4912560d537bd7689c45ee4daaefd344406cf6fe5b8219
//META: purpose=A SwitchStmt whose body contains only a `default:` marker (no CaseStmt) dispatches every condition value to the default body; the documented "marker-slicing" of the body still works at the lower edge of one section.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: SwitchStmt with one CaseStmtBase entry (the DefaultStmt
// alone). The doc says the body is "sliced into per-case blocks";
// with only a default block, every condition value hits the default.

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

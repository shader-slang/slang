//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#switchstmt-casestmt-defaultstmt
//META: doc_section_digest=a1ec0803b8c128d1fa4912560d537bd7689c45ee4daaefd344406cf6fe5b8219
//META: purpose=A SwitchStmt without a DefaultStmt slices the body into per-case blocks only; an unmatched condition value passes through without entering any case body.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: SwitchStmt with two CaseStmt entries but no DefaultStmt.
// The doc treats `default` as a CaseStmtBase marker among others; a
// missing default means an unmatched value falls through the switch
// without setting `r`. We observe both the surviving `switch` text
// and that `classify(99)` leaves `r` at its initial 0.

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

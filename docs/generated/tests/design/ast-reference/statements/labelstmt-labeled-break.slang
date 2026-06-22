//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#labelstmt-and-breakstmttargetlabel
//META: doc_section_digest=b99c202f30122b46c011fe06619b9a5c3611a4e6fc931e95c05d3861323a2ce2
//META: purpose=A LabelStmt attaches a label token to an inner statement; a `BreakStmt` with a matching `targetLabel` breaks out of the labeled (non-innermost) loop rather than the innermost one.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "BreakStmt::targetLabel optionally names the enclosing
// labeled loop or switch to break out of. Resolution of `targetLabel`
// to a `BreakableStmt::uniqueID` is done by the checker." We observe
// labeled break by escaping a 2-deep loop nest from the inner loop:
// the outer loop must not get a chance to start its second iteration.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int outerHits = 0;
    int innerHits = 0;
outer:
    for (int i = 0; i < 3; i = i + 1)
    {
        outerHits = outerHits + 1;
        for (int j = 0; j < 10; j = j + 1)
        {
            innerHits = innerHits + 1;
            if (j == 1) { break outer; }
        }
    }
    // outer body entered once before break outer fires
    // CHECK: outerHits=1
    printf("outerHits=%d\n", outerHits);
    // inner ran j=0,1 then broke outer => 2
    // CHECK: innerHits=2
    printf("innerHits=%d\n", innerHits);
}

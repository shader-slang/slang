//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#labelstmt-breakstmt-continuestmt-and-discardstmt
//META: doc_section_digest=7c61fa1b4009ab648f41bc90151176c2c3ef804455b39f0ca18a8fb8d9a62d07
//META: purpose=A LabelStmt attaches a label token to an inner statement; a `BreakStmt` with a matching `targetLabel` breaks out of the labeled (non-innermost) loop rather than the innermost one.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a label attached to a loop can be named by `break`, so the
// jump leaves the labeled outer loop rather than the innermost one. The
// CHECKs assert the outer body ran once and the inner ran twice, which
// is only reachable if the labeled break escaped both loops.

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

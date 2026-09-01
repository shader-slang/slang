//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#labelstmt-breakstmt-continuestmt-and-discardstmt
//META: doc_section_digest=7c61fa1b4009ab648f41bc90151176c2c3ef804455b39f0ca18a8fb8d9a62d07
//META: purpose=A ContinueStmt restarts the nearest enclosing loop; a `continue` in an inner loop skips only the inner iteration's remainder while the outer loop still completes all of its iterations.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `continue` binds to the *nearest* enclosing loop, not any
// outer one. Two nested for-loops: the inner `continue` fires on even
// j, so the inner tail runs twice per outer pass (odd j in 0..3),
// while the outer loop still completes all 3 passes. The CHECK
// asserts outer=3 (continue never restarted the outer loop) and
// innerTails=6 (2 tails x 3 outer passes).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int outerRuns = 0;
    int innerTails = 0;
    for (int i = 0; i < 3; i = i + 1)
    {
        outerRuns = outerRuns + 1;
        for (int j = 0; j < 4; j = j + 1)
        {
            if ((j % 2) == 0) { continue; }
            innerTails = innerTails + 1;
        }
    }
    // CHECK: outer=3 innerTails=6
    printf("outer=%d innerTails=%d\n", outerRuns, innerTails);
}

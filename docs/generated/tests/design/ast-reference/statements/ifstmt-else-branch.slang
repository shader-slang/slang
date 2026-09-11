//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#ifstmt
//META: doc_section_digest=a6c1740eeba3eb150d93ff559a355c4b73decc79a2b471fc7f1203bf5d838824
//META: purpose=An IfStmt holds predicate, positive and negative statements; only the branch matching the predicate is executed, and the unchosen branch's side effects are not observed.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies exactly one arm of an `if`/`else` executes per evaluation.
// Each arm bumps a shared counter by a distinct amount, so the final
// counter value names the arms that ran; the CHECK asserts 11, the sum
// of the true-arm and false-arm contributions only.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int counter = 0;
    if (1 == 1) { counter = counter + 10; } else { counter = counter + 100; }
    if (1 == 2) { counter = counter + 1000; } else { counter = counter + 1; }
    // 10 + 1 = 11
    // CHECK: counter=11
    printf("counter=%d\n", counter);
}

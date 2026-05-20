//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#ifstmt
//META: doc_section_digest=10062280ed5cf8f826912bbfaa3a2c63188ff4d4c316ffda45574aab2cc4dc2c
//META: purpose=An IfStmt holds predicate, positive and negative statements; only the branch matching the predicate is executed, and the unchosen branch's side effects are not observed.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The IfStmt section documents `predicate`, `positiveStatement`, and
// `negativeStatement`. We side-effect a single counter from each
// branch and observe that exactly one branch fired per evaluation, so
// a checker that fired both arms would inflate the count.

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

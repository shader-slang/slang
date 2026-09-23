//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=90f68d89058e95e927dbc5ffd876802120cff1e5fe6639465168476f05517709
//META: purpose=A `for (init; cond; update) { body }` form parses into a Stmt with three distinct sub-expressions and a body block.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Sum 0..3 with a counted for-loop. The accumulated `s` only equals
// 6 if all three clauses (init, cond, update) were parsed into their
// proper slots on the ForStmt and the body parsed as a block.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int s = 0;
    for (int i = 0; i < 4; i = i + 1) { s = s + i; }
    // CHECK: s=6
    printf("s=%d\n", s);
}

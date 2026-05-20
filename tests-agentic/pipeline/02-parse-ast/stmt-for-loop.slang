//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=533753bf58b03097d708a72394179634eea26760dccf152ebe2bcd53f0cfb0c0
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

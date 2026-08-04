//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=5749d9886287d8389af87cb5a6a536a28da48a421ac9a27f86809fca6a7b5676
//META: purpose=Assignment is parsed as a right-associative Expr, so `a = b = c = 7` reduces as `a = (b = (c = 7))` and all three names receive the value.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// If assignment were left-associative the leftmost `=` would bind
// first and the chain would fail to parse (or assign different
// values). The fact that all three variables end up holding 7
// confirms the right-associative Expr shape.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a; int b; int c;
    a = b = c = 7;
    // CHECK: vals=7 7 7
    printf("vals=%d %d %d\n", a, b, c);
}

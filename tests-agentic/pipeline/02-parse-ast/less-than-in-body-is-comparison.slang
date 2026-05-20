//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=6cf4f0017f8ed0ca4ddf4b4f37de1831b62091dde8e6a8351a061ad3fe939266
//META: purpose=When the generic-application parse does not commit cleanly the parser rolls back and parses `<` as the less-than operator, so `a < b` in a body context yields a boolean comparison Expr.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `a < b` is the rollback case: the parser tries a generic-argument
// parse against the local variable `a`, fails to commit, and produces
// a less-than expression instead. The boolean result drives a `?:`
// select that proves the comparison parsed.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 5;
    int b = 9;
    int c = (a < b) ? 1 : 0;
    // CHECK: c=1
    printf("c=%d\n", c);
}

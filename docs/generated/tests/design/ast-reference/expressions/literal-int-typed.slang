//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=An IntegerLiteralExpr carries a parsed integer value that survives to runtime as that integer.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `12345` is parsed as IntegerLiteralExpr with value 12345. The
// LiteralExpr family stores the parsed value; printing it back proves
// the value made it through to evaluation unchanged.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: v=12345
    printf("v=%d\n", 12345);
}

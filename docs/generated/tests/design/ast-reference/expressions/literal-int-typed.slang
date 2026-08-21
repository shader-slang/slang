//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
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

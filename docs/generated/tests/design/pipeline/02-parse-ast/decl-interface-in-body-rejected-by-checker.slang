//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=35a0377e8327c387000594e690000a91232b821b59188e50e4c846f99b749334
//META: purpose=An `interface` written inside a function body is rejected by the checker nesting rule with E31400, not by the parser.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second, independent nesting validation: the statement parser
// keeps an aggregate type, but the checker's child/parent table does not admit
// an `interface` inside a function, so E31400 is reported (the parser-side
// E30102 is not). CHECK pins the code and the wording.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int main()
{
    interface I { int f(); }
//CHECK:      ^ error E31400
//CHECK:      ^ interface declaration is not allowed inside function.
    return 0;
}

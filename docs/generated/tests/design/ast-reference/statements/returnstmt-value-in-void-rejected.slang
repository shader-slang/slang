//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/statements.md#returnstmt
//META: doc_section_digest=f1e8b66abbff0395f77b1ea4055b32d0423ac884166270a7cbff04f0383235db
//META: purpose=The checker matches a ReturnStmt's expression against the enclosing function's declared return type; `return 1;` in a `void` function is rejected because `int` does not implicitly convert to `void`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the return-type matching runs at the void boundary too:
// only a null expression is admissible for a `void` return, so a
// `return 1;` in a `void` function must fail the checker's match of
// the expression against the declared return type. The CHECK pins
// E30019 on the returned literal; a checker that skipped the match
// for void functions would accept this silently.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void f()
{
    return 1;
/*CHECK:
           ^ type mismatch in expression
           ^ expected an expression of type 'void', got 'int'
           ^ explicit conversion from 'int' to 'void' is possible
*/
// CHECK: E30019
}

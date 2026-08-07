//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/statements.md#returnstmt
//META: doc_section_digest=f1e8b66abbff0395f77b1ea4055b32d0423ac884166270a7cbff04f0383235db
//META: purpose=The checker matches a ReturnStmt's expression against the enclosing function's declared return type; returning a struct value from an `int` function is rejected with a type mismatch.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the checker rejects a return whose expression's type does
// not match the function's declared return type. A function declared
// to return `int` returns a struct value that has no conversion to
// `int`; the CHECK pins the E30019 type-mismatch diagnostic on the
// returned expression, which fires only if the checker compared the
// expression's type against the declared return type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S
{
    int a;
}

int f()
{
    S s;
    return s;
/*CHECK:
           ^ type mismatch in expression
           ^ expected an expression of type 'int', got 'S'
*/
// CHECK: E30019
}

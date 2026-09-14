//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#returnstmt
//META: doc_section_digest=c25c0b38fe3f374b6c53bf5d39f45dbef8f00f160edf334a7927f62cea89fd70
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

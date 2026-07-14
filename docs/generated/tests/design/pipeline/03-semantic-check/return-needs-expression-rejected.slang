//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Statement checking validates `return` against the function's return type; a bare `return` in a non-void function is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// slang-check-stmt.cpp is documented to handle return-type
// validation. Verify the user-visible consequence: a bare `return`
// in an int-returning function is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int foo()
{
    return;
/*CHECK:
    ^^^^^^ 'return' should have an expression
*/
// CHECK: E30006
}

void main() { int z = foo(); }

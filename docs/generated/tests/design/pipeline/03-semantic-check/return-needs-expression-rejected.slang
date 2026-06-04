//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
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

//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Expression checking enforces lvalue-ness; assigning to a non-lvalue expression is rejected with diagnostic 30011.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// slang-check-expr.cpp is documented to handle "type inference,
// lvalue-ness, conversions". Verify the lvalue-ness check by
// trying to assign to a temporary expression.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 5;
    (x + 1) = 10;
//CHECK:    ^ left of '=' is not an l-value
//CHECK:    ^ left of '=' is not an l-value.
// CHECK: E30011
}

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=955b45bda31b26608aa302d1b2e04635daf73a8cc4ddd4784579a780f4e536d1
//META: purpose=Expression checking enforces lvalue-ness: assigning to an arithmetic expression is rejected with E30011.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "lvalue-ness" concern the expression checker owns. `a + 1`
// denotes a value, not a storage location, so the assignment must be
// rejected before any lowering happens; CHECK pins E30011 on the assignment.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int a = 1;
    a + 1 = 2;
//CHECK: E30011
}

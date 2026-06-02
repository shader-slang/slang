//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:37:14+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-this.md#mutability
//META: doc_section_digest=2822c7f6b30708a773dbe170117103457227a510de3a625a9ca4bed55f2b62fe
//META: purpose=A call expression is never an l-value; assigning to one is rejected with E30011.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "A call expression is never an l-value."
// Directly assigning to the result of a call must be rejected at compile time (E30011).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int makeVal() { return 5; }

void main()
{
    makeVal() = 10;
//CHECK:      ^ left of '=' is not an l-value.
}

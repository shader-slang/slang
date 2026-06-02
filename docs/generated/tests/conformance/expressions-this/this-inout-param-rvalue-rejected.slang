//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:37:14+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-this.md#mutability
//META: doc_section_digest=2822c7f6b30708a773dbe170117103457227a510de3a625a9ca4bed55f2b62fe
//META: purpose=Passing an r-value expression as an `inout` parameter argument is rejected because the doc states `in out` arguments must be l-values.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "The argument expressions corresponding to any `out` or `in out` parameters
// of the callee must be l-values." A call-expression result is an r-value; passing it to
// an `inout` parameter must be rejected at compile time (E30047).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void addOne(inout int x) { x = x + 1; }
int makeVal() { return 5; }

void main()
{
    addOne(makeVal());
//CHECK:          ^ argument must be l-value
}

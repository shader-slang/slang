//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#nodes
//META: doc_section_digest=29d86296deb73a57a4f207507e16ba645a6bf2e722162db8157e7554d2cf8004
//META: purpose=`const` makes a local variable immutable; assigning to it is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `const` is not merely decorative on a local: it makes the binding
// immutable. The variable is initialised once and then assigned again, and CHECK
// pins the not-modifiable error code, proving the checker treats the `const`
// declaration as a non-l-value target.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    const int x = 7;
    x = 42;
    //CHECK: error
    //CHECK: E30011
}

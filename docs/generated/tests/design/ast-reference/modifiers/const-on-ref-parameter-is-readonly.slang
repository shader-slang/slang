//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:34:09Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#nodes
//META: doc_section_digest=29d86296deb73a57a4f207507e16ba645a6bf2e722162db8157e7554d2cf8004
//META: purpose=On a `__ref` parameter, `const` is the legacy alias of `__ref_readonly`: assigning through it is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `ConstModifier` row: `const` written on a `__ref` parameter
// makes the parameter read-only in the callee, so the write is rejected. CHECK
// pins the not-an-l-value error code, which shows the `const` took effect
// rather than being ignored.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void f(const __ref int x)
{
    x = 1;
    //CHECK: error
    //CHECK: E30011
}

void main()
{
    int a = 0;
    f(a);
}

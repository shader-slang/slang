//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=Warnings draw ids from the same integer namespace as errors: comma-operator-used-in-expression renders E41024.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that warnings draw ids from the same integer namespace as errors.
// A comma operator in a value context triggers comma-operator-used-in-expression;
// the caret-anchored CHECK pins the rendered code E41024 at the comma's column.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):
int main()
{
    int x = (1, 2);
//CHECK:      ^ E41024
//CHECK:      ^ comma operator used in expression
    return x;
}

//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=d8418d47e63070ba0cf68467df4c96fca1462a9b09b58fcda9b0f2dcaac6484b
//META: purpose=With no severity override registered in the sink, the comma-operator warning E41024 reaches the user unchanged.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Positive control for the suppression tests: with no override registered the
// same comma-operator expression that `#pragma warning(disable: 41024)`
// silences does reach the user. CHECK pins the code and message at the comma's
// column, so the suppression tests are known to be changing behaviour.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):
int main()
{
    int x = (1, 2);
//CHECK:      ^ E41024
//CHECK:      ^ comma operator used in expression
    return x;
}

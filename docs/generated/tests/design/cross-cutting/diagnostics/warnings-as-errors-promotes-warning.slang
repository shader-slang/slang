//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=45a1f9ffaa6abf3626a600faa360e7aa4723d4e67c65508783a32e11ac892cf2
//META: purpose=The final `TreatWarningsAsErrors` step of `getEffectiveMessageSeverity` promotes warning 41024 to `error` when `-warnings-as-errors 41024` selects it by id.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the last step of the effective-severity computation: a warning that
// survives the earlier steps is promoted to Error when warnings-as-errors
// selects it. The comma-operator warning is compiled with
// `-warnings-as-errors 41024`; the annotation asserts the reported severity
// word at that column is `error` rather than `warning`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -warnings-as-errors 41024
int main()
{
    int x = (1, 2);
//CHECK:      ^ error
//CHECK:      ^ comma operator used in expression
    return x;
}

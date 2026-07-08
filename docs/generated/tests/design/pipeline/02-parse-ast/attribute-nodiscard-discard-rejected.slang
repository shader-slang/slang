//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=85307207e3b8f9c4a6840f45c1896c59f8f18e3bce3e9753d2b831e1d7ea443a
//META: purpose=A call to a `[NoDiscard]` function whose result is discarded in an expression statement is diagnosed with E30059, showing the attribute parsed into its dedicated Attribute subclass.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's named example of an attribute with its own
// semantics: [NoDiscard] parses as a dedicated attribute and the
// checker errors on a result-discarding call — the exact context the
// doc names (an expression statement). CHECK pins the code E30059.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[NoDiscard]
int produce()
{
    return 42;
}

void main()
{
    produce();
//CHECK:   ^ error E30059
//CHECK:   ^ the result of calling 'produce' is discarded; this function is marked '[NoDiscard]'.
}

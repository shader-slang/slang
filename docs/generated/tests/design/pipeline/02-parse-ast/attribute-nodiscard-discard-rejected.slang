//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=70d77929d730a0398ad6cf62c037992abc0cd9b4600f071779b6380151df5290
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

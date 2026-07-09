//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: subscripting a scalar value renders E30013 with the documented "no subscript declarations found for type" message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `subscript-non-array` (30013) to its rendered code.
// `int` has no `operator[]` so subscripting it triggers the
// diagnostic with the type interpolated.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int main()
{
    int a = 5;
    return a[0];
//CHECK:    ^ E30013
//CHECK:    ^ no subscript declarations found for type 'int'
}

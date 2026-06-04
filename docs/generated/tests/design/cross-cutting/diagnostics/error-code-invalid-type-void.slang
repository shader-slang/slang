//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: declaring a local variable of type `void` renders E30009 with the documented "invalid type 'void'" message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `invalid-type-void` (30009) to its rendered code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    void v;
/*CHECK:
    ^^^^ E30009
    ^^^^ invalid type 'void'.
*/
}

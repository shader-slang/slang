//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: assigning to a non-l-value expression renders E30011 with the documented "left of '=' is not an l-value" message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `assign-non-lvalue` (30011) to its rendered code by writing
// to a binary expression that produces an rvalue.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int main()
{
    int a = 5;
    a + 1 = 10;
//CHECK:  ^ E30011
//CHECK:  ^ left of '=' is not an l-value.
    return a;
}

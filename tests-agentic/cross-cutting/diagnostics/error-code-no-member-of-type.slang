//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: accessing a non-existent struct member renders E30027 with the documented "member not found" message and the member-name interpolated.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `no-member-of-name-in-type` (30027) to its rendered code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
struct S { int x; };
void main()
{
    S s;
    s.y = 3;
/*CHECK:
      ^ E30027
      ^ 'y' is not a member of 'S'.
*/
}

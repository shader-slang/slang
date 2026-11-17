//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=A missing struct member renders E30027 with the member name interpolated.
//META: intent=functional
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

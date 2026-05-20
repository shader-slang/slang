//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-legacy-diagnostic
//META: doc_section_digest=04e6b41a5ed7b7f40deee827fed8dac1f618dc77bdfc81ebd67f98a5810bb953
//META: purpose=Legacy diagnostics interpolate ~param fields into the rendered text; the undeclared name appears quoted in the span message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc shows legacy `~param` and `~param:Type` interpolation. The
// undefined-identifier diagnostic uses `~name:Name`. Verify that the
// specific undeclared name appears quoted in the rendered message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
void main()
{
    int a = mysteryIdent;
//CHECK:    ^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^ undefined identifier 'mysteryIdent'.
}

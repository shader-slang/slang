//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=a2c5b6b64f38a8b7fd3692290be87fe1f26f4a31a5f12e076913b5c584acc7f3
//META: purpose=Boundary: `-warnings-as-errors all` promotes every surviving warning to `error`, not just one selected id.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `all` boundary of the warnings-as-errors promotion: rather than
// naming one id, the request covers every warning that survives the earlier
// severity steps. Two unrelated warnings are triggered in one file and the
// annotations assert both are reported at severity `error`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-warnings-as-errors all
int main()
{
    int x = (1, 2);
//CHECK:      ^ error
//CHECK:      ^ comma operator used in expression
    int enum = x;
//CHECK:^^^^ error
//CHECK:^^^^ is a type keyword
    return enum;
}

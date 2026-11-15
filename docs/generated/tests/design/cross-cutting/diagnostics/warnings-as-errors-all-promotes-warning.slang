//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=3ea14a1946d125d6e51f1168520ddc4e04afd09127febe10b5079af9e30b1816
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

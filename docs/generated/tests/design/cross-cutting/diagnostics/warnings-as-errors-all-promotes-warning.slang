//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=c2dc29952382274ed0bbb3fd75b8dda850859c3b85a57e5b07f0cbaf0a1abf4b
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

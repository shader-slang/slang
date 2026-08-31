//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=233c54717a586cdba7e515d0d6fbb4c7e6363f272d4b60a4aed8b9997a298ad9
//META: purpose=Modifier validation rejects a redundant repetition of the same modifier on one declaration (E31202).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "mutually exclusive / redundant combinations" rule. Repeating
// `static` on one declaration is not additive, so the second occurrence is
// reported against the first. CHECK pins E31202 on the repeated modifier.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static static int g = 1;
//CHECK: E31202

void main()
{
}

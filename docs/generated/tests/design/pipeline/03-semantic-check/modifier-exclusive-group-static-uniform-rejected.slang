//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=233c54717a586cdba7e515d0d6fbb4c7e6363f272d4b60a4aed8b9997a298ad9
//META: purpose=`static` and `uniform` share one conflict group, so a declaration carrying both is rejected (E31202).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second named multi-member conflict group. The two modifiers are
// unrelated spellings that nonetheless compete for the same storage decision,
// so declaring both must be an error rather than an intersection of the two.
// CHECK pins E31202 and the message naming `uniform` against `static`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static uniform int g = 1;
//CHECK: E31202
//CHECK: modifier 'uniform' is redundant or conflicting with existing modifier 'static'

void main()
{
}

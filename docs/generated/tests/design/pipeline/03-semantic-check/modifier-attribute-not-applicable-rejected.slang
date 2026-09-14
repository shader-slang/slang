//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=233c54717a586cdba7e515d0d6fbb4c7e6363f272d4b60a4aed8b9997a298ad9
//META: purpose=Modifier validation rejects an attribute placed on a declaration kind it does not apply to (E31002).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "which modifiers are allowed on which decls" rule.
// `[numthreads]` is a compute-entry-point attribute; attaching it to a struct
// has no meaning, and the check is what turns that into an error rather than
// a silently ignored annotation. CHECK pins E31002 on the attribute name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[numthreads(1, 1, 1)]
struct S
{
    int x;
}
//CHECK: E31002

void main()
{
}

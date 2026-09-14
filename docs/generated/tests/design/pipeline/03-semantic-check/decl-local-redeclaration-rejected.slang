//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=955b45bda31b26608aa302d1b2e04635daf73a8cc4ddd4784579a780f4e536d1
//META: purpose=Decl checking rejects two locals with the same name declared in the same block as a conflicting declaration (E30200).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same "Decl checking" collision rule at the statement shape, so
// the claim is not carried by the type-declaration case alone. Both `v`
// declarations live in one block; CHECK pins E30200 plus the note naming the
// earlier binding.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int v = 1;
    int v = 2;
//CHECK: E30200
//CHECK: see previous declaration of 'v'
}

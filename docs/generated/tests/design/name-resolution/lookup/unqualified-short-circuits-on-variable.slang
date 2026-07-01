//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=3b394bf839041f0d86487cb14f22d3c9442c777b87a2bbf98421edb81222a98a
//META: purpose=Step 6 short-circuits on a non-overloadable hit, so an inner local variable stops the outward scope walk and a same-named outer value is never seen.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "After visiting one scope and its siblings, if the result is valid
// and either non-overloaded or not overloadable, lookup stops
// walking outwards ... Functions and generics are overloadable, so
// they continue to accumulate candidates from outer scopes; types
// and variables do not."  Verify that a local variable short-
// circuits: the outer value is never visible.

//TEST:INTERPRET(filecheck=CHECK):

static const int K = 999;

void main()
{
    int K = 7; // shadows file-scope K; lookup short-circuits here.
    // CHECK: r=7
    printf("r=%d\n", K);
}

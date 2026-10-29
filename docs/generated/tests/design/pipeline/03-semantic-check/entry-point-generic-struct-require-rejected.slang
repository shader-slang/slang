//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=cb835744c66906760cd040d4b867053da3041145426b47d4afb564e26340984f
//META: purpose=A user-defined generic struct in an entry-point signature has its `[require(...)]` validated against the target, so `Foo<int>` requiring an unavailable capability is rejected with E36107.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the entry-point-scoped generic-struct capability check. The
// requirement sits on `Foo`, and the signature spells the specialization
// `Foo<int>`; without the dedicated walk over the signature that requirement
// would be dropped and the shader would compile silently for a target that
// cannot provide it. CHECK pins E36107 and the note naming `Foo` as the
// contributing use.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -emit-spirv-directly -entry main -stage vertex

[require(cpp)]
struct Foo<T>
{
    T value;
}

[shader("vertex")]
void main(Foo<int> a)
{
}
//CHECK: E36107
//CHECK: see using of 'Foo'

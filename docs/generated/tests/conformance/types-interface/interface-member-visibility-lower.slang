//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=The visibility of a struct's interface member implementation may not be higher than the requirement, but may be lower; a struct implementing a public requirement with `internal` visibility is accepted.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that implementing an interface member with lower visibility than
// the requirement is accepted. LowerVisImpl marks getValue() as `internal`
// (lower than the default public requirement); CHECK confirms the file
// compiles and emits a valid entry point.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

interface IPublicReq
{
    int getValue();
}

// internal is LOWER than default (public) visibility — accepted
struct LowerVisImpl : IPublicReq
{
    internal int getValue() { return 42; }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}

// CHECK: void main

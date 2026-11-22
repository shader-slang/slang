//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=9cf2018f7631fb1edb894cee46db1dd141344017dc943113e8662661b88df42c
//META: purpose=A synthesized differential type propagates the parent member's visibility; a `public` function may name the auto-synthesized `Differential` of a `public` struct in its signature.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the auto-synthesized `Primal.Differential` carries the parent
// struct's `public` visibility rather than an unrelated default. A
// `public` function whose signature names `Primal.Differential` only
// passes checkVisibility (no use-of-less-visible-type rejection) if the
// synthesized type's visibility propagated up to `public`. CHECK asserts
// the program checked, lowered, and ran through that signature.

//TEST:INTERPRET(filecheck=CHECK):

public struct Primal : IDifferentiable
{
    public float x;
}

public Primal.Differential passThrough(Primal.Differential d)
{
    return d;
}

void main()
{
    Primal.Differential d = Primal.dzero();
    d = passThrough(d);
    // CHECK: ok=1
    printf("ok=%d\n", 1);
}

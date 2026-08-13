//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=5af69869d1bb41b9ec6da86b628cac4884b654398df91d571e67dbe22a034ace
//META: purpose=A synthesized requirement witness takes Min(parent visibility, requirement visibility); an internal struct can satisfy an explicitly-public interface requirement without a higher-than-parent violation.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the requirement-satisfaction synthesizer caps a synthesized
// member's visibility at Min(parent, requirement). The requirement is
// explicitly `public` while the conforming struct is `internal`; the
// property witness synthesized from the plain field must be capped at
// `internal`, so the program passes checkVisibility (no
// higher-than-parent rejection) and the generic read through the
// interface succeeds. CHECK asserts the value flows through the
// synthesized accessor.

//TEST:INTERPRET(filecheck=CHECK):

public interface IHasValue
{
    public property value : int { get; }
}

internal struct Wrapped : IHasValue
{
    internal int value;
    internal __init(int v) { value = v; }
}

internal int readThrough<T : IHasValue>(T t)
{
    return t.value;
}

void main()
{
    // CHECK: value=42
    printf("value=%d\n", readThrough(Wrapped(42)));
}

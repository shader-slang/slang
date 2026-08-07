//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:40Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=25ecf192e380725d185511e33ffea330856108b33c0f2d39992576f85bcc7cd3
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

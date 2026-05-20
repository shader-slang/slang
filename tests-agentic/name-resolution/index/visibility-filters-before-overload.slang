//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/name-resolution/index.md#flow-diagram
//META: doc_section_digest=5bf0b1723f5c95f0efe882446074467f222ba31a7cb99b373e5cfcaa35edc75e
//META: purpose=In the documented phase order, the visibility filter precedes overload resolution, so a more-specific but inaccessible overload does not win over a less-specific but visible one.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The flow diagram orders the phases as: raw LookupResult -> (see
// visibility.md) -> filtered LookupResult -> (see
// overload-resolution.md) -> ranked. If visibility filtering ran
// AFTER overload ranking, the exact-type-matching PRIVATE
// candidate would be picked by the ranker and then rejected,
// producing a visibility diagnostic. Because filtering runs
// FIRST, the private candidate is gone before the ranker sees the
// set, so the public (less specific, requires int->float
// conversion) candidate is used silently.

//TEST:INTERPRET(filecheck=CHECK):

public struct S
{
    // Less-specific public candidate: takes float, would lose the
    // overload rank to an exact-int candidate.
    public int classify(float x)
    {
        return 1;
    }

    // More-specific PRIVATE candidate: exact-type match for the
    // int argument below; would win overload ranking if visible.
    private int classify(int x)
    {
        return 2;
    }
}

void main()
{
    S s;
    // Because visibility filters before overload, the private
    // exact-int overload is removed; the call resolves to the
    // public float overload via int->float conversion, returning
    // 1. No diagnostic is emitted.
    // CHECK: r=1
    printf("r=%d\n", s.classify(7));
}

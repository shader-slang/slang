//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/name-resolution/index.md#flow-diagram
//META: doc_section_digest=5bf0b1723f5c95f0efe882446074467f222ba31a7cb99b373e5cfcaa35edc75e
//META: purpose=The four documented phases compose: scope walk yields a raw LookupResult, the visibility filter passes it through, and overload resolution picks the single visible candidate.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The flow diagram shows: identifier -> scope walk -> raw
// LookupResult -> visibility filter -> filtered LookupResult ->
// overload resolution -> DeclRef. This test composes all of them:
// `pick` is referenced unqualified (scope walk through file
// scope), has two same-name overloads (raw LookupResult is a
// set), both are public (visibility lets both through), and one
// matches by arity (overload resolution picks the unique winner).

//TEST:INTERPRET(filecheck=CHECK):

public int pick(int a)
{
    return a + 1;
}

public int pick(int a, int b)
{
    return a * b;
}

void main()
{
    // Only the 2-argument candidate matches; the 1-argument
    // candidate is eliminated during arity-based overload
    // narrowing AFTER lookup + visibility have produced both.
    // CHECK: r=20
    printf("r=%d\n", pick(4, 5));
}

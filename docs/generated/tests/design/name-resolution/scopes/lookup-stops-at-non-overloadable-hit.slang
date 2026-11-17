//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-walking-order-during-lookup
//META: doc_section_digest=1a49dc286532d677a254b9cf02b7dfa3065f611704c034f59d6210cab0b32c1b
//META: purpose=Lookup stops at the first level with a non-overloadable hit, so an inner declaration shadows an outer one of a different kind even when the outer one would otherwise be a better match.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Step 4 of the walk: once a level has produced a result that is not
// overloaded and not of an overloadable kind, lookup does not go on to the
// parent. Inside namespace N the variable `v` is found and the outer
// function `v` is never consulted, so `v` is the int 7.

//TEST:INTERPRET(filecheck=CHECK):

int v() { return 100; }

namespace N {
    static const int v = 7;
    int get() { return v; }
}

void main()
{
    // CHECK: g=7
    printf("g=%d\n", N::get());
}

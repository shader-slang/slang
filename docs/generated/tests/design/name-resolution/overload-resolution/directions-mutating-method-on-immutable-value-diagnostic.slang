//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=471a235af8be78d2eb5c62c0f69c7e4037797818d1630eb02a08fcdaae148425
//META: purpose=TryCheckOverloadCandidateDirections checks the mutability of the effective this parameter and rejects a [mutating] method invoked on an immutable base.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the direction step's actual job: it gates the implicit `this`
// parameter, not general argument l-value-ness. `look` is non-mutating, so its
// `this` is immutable, and calling the `[mutating]` `bump` from inside it is
// rejected. CHECK pins the mutating-method diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    int v;
    [mutating] int bump() { v = v + 1; return v; }
    int look() { return bump(); }
//CHECK:                    ^ mutating method 'bump' cannot be called on an immutable value
}

void main()
{
    S s;
    s.v = 1;
    printf("%d\n", s.look());
}

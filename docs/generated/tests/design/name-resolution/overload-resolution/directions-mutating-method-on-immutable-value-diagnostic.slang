//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=211bc03cd9a092841d1719f62552fc549d768b7b9dc4ea9b6c978e0acc8daa2b
//META: purpose=TryCheckOverloadCandidateDirections checks the mutability of the implicit this parameter and rejects a [mutating] method invoked on an immutable base.
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

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#breadcrumbs
//META: doc_section_digest=48d49844973ad09b45a6acd124d2e1aaf5f16e02ec5ca18dda08dfb5f469bf11
//META: purpose=A member reached only through an unchecked optional conformance is removed after lookup, and the empty result is reported as the required-constraint-is-not-checked diagnostic (E30403).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative half of the optional-constraint filter. This
// is the same generic as
// optional-constraint-member-visible-after-is-check.slang with the `is`
// guard removed, so the only navigation step to `code()` carries an
// unchecked optional witness and the otherwise-valid result is emptied.
// CHECK annotates the message at the member and pins the code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IPrintable { int code(); }

int tryIt<T>(T v) where optional T : IPrintable
{
    return v.code();
//CHECK:     ^^^^ the constraint providing 'code' is optional and must be checked with an 'is' statement before usage
// CHECK: E30403
}

struct P : IPrintable { int code() { return 55; } }

void main()
{
    P p;
    printf("r=%d\n", tryIt(p));
}

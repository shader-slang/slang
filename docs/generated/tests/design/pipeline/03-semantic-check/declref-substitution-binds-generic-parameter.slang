//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#name-lookup-and-declref
//META: doc_section_digest=974c295d7822368d7371b23d0dae553135ae7f5870e74a61cb71417386c7531f
//META: purpose=A DeclRef records the substitution that binds a generic parameter, so two instantiations of one generic function behave as different concrete types.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that name resolution yields a decl *plus* a substitution rather
// than a bare decl. `describe` is one declaration; the two call sites differ
// only in the type bound to `T`, and the interface method each dispatches to
// is chosen by that binding. CHECK asserts both instantiations returned their
// own value, which requires the substitution to be part of the reference.

//TEST:INTERPRET(filecheck=CHECK):

interface INamed
{
    int code();
}

struct Red : INamed
{
    int code() { return 1; }
}

struct Blue : INamed
{
    int code() { return 2; }
}

int describe<T : INamed>(T v)
{
    return v.code();
}

void main()
{
    Red r;
    Blue b;
    // CHECK: a=1 b=2
    printf("a=%d b=%d\n", describe(r), describe(b));
}

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#partial-generic-application
//META: doc_section_digest=e06ed409a53cec273f1b68cefb6951235a8d59457bb201304104fe7026c8e802
//META: purpose=A generic candidate whose parameters are all pinned by the call arguments is fully specialized rather than left partially applied.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fully-closed end of the partial-application spectrum: the single
// argument determines T, so no PartiallyAppliedGenericExpr survives and the
// call is an ordinary specialized invocation. CHECK asserts the returned value
// round-trips at the argument's type.

//TEST:INTERPRET(filecheck=CHECK):

T identity<T>(T x) { return x; }

void main()
{
    int v = identity(42);
    // CHECK: v=42
    printf("v=%d\n", v);
}

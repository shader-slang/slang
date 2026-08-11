//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=5af69869d1bb41b9ec6da86b628cac4884b654398df91d571e67dbe22a034ace
//META: purpose=A `GenericDecl` takes its visibility from the inner decl; a public generic function is reachable through the public-on-inner classification.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `getDeclVisibility` for a `GenericDecl` returns the visibility of
// its `inner` decl. Marking the inner function `public` makes the
// generic function callable.

//TEST:INTERPRET(filecheck=CHECK):

public int identity<T : IInteger>(T x) where T == int { return int(x); }

void main()
{
    // CHECK: id=11
    printf("id=%d\n", identity(11));
}

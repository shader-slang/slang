//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/name-resolution/visibility.md#generic-parameters-accessors-and-synthesized-members
//META: doc_section_digest=ec08120339d3d14caf888d6a0bddd84667a5c4a5996a43c90cd4d9dd60c37807
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

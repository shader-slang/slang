//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#invokeexpr-and-the-call--operator--cast-unification
//META: doc_section_digest=bed8880e5d3d3d67324d1cc51b8508d2d1f9ce1f6e5e3f3eadf4c071f728a402
//META: purpose=Verifies an InvokeExpr with zero arguments — `f()` — calls the function and returns its result.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary for InvokeExpr.arguments: the empty list.
// `f()` must construct an InvokeExpr with no args and dispatch
// correctly to the zero-arg overload.

//TEST:INTERPRET(filecheck=CHECK):

int five() { return 5; }

void main()
{
    // CHECK: r=5
    printf("r=%d\n", five());
}

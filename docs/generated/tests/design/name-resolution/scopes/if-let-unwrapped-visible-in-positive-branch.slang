//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=ea81eee9c248c045a204cf03769d29438ea6b24a6cbd441a4c42b3efc37f9f53
//META: purpose=The `if (let x = ...)` desugaring creates a ScopeDecl for the user-visible unwrapped variable inside the positive branch, so the unwrapped name resolves there.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unwrapped variable of an `if (let ...)` resolves inside the
// positive branch: the parser pushes a ScopeDecl carrying `v` for that
// branch only. Two Optionals are unwrapped in the same function, one with a
// payload and one without. CHECK asserts the payload is read through `v` in
// the first case, and that the branch is not entered in the second, so the
// scope that carries `v` is confined to the positive branch.
// The Optionals are locals rather than call arguments on purpose: a `none`
// argument crossing a call boundary hits a separate slangi VM defect that
// would mask the scoping observation this test is about.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Optional<int> present = Optional<int>(41);
    int a = -1;
    if (let v = present)
    {
        a = v;
    }
    // CHECK: a=41
    printf("a=%d\n", a);

    Optional<int> absent = none;
    int b = -1;
    if (let v = absent)
    {
        b = v;
    }
    // CHECK: b=-1
    printf("b=%d\n", b);
}

//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=677c751462cdb5ae5189c32a0f57ea5105199a379d69be08450135057a10b2db
//META: purpose=The `if (let x = ...)` desugaring creates a ScopeDecl for the user-visible unwrapped variable inside the positive branch, so the unwrapped name resolves there.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unwrapped variable of an if-let resolves inside the
// positive branch: parseIfLetStatement pushes a ScopeDecl carrying `v`
// for that branch. The test unwraps a present Optional and returns `v`;
// CHECK asserts the unwrapped value (not the fallback) is printed, which
// only happens if `v` resolved inside the branch.

//TEST:INTERPRET(filecheck=CHECK):

int unwrapOrNegative(Optional<int> opt)
{
    if (let v = opt)
    {
        return v;
    }
    return -1;
}

void main()
{
    int a = unwrapOrNegative(Optional<int>(41));
    // CHECK: a=41
    printf("a=%d\n", a);
    int b = unwrapOrNegative(none);
    // CHECK: b=-1
    printf("b=%d\n", b);
}

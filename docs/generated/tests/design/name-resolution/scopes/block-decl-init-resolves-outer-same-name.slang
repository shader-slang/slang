//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
//META: purpose=A block-local declaration is still hidden from lookup while its own initializer is checked, so a same-named outer local is what the initializer resolves to.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary of the per-decl hidden-from-lookup flag inside a
// block: the flag is only cleared once the checker has walked past the
// declaration statement, so the initializer of the inner `x` cannot see the
// inner `x` and resolves to the enclosing one instead. The inner block
// declares `int x = x + 10` while an outer `int x = 1` is live; CHECK
// asserts `inner=11` (outer 1 plus 10, not a self-reference) and that the
// outer binding is unchanged after the block.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 1;
    {
        int x = x + 10;
        // CHECK: inner=11
        printf("inner=%d\n", x);
    }
    // CHECK: outer=1
    printf("outer=%d\n", x);
}

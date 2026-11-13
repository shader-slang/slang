//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=62d41f734409e853620e442a2938448e5d7be850d748cdbabc3e948d9882271a
//META: purpose=A WhileStmt with a constant-true predicate iterates indefinitely; a body-internal BreakStmt terminates the LoopStmt at the documented iteration.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: a `while` whose predicate is literally `true`. Verifies the
// loop is still breakable, so a `break` in the body terminates it at
// the expected iteration. The CHECK asserts exactly four iterations
// ran.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int n = 0;
    while (true)
    {
        n = n + 1;
        if (n == 4) { break; }
    }
    // CHECK: n=4
    printf("n=%d\n", n);
}

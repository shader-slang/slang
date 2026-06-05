//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-walking-order-during-lookup
//META: doc_section_digest=e8fc5df08f9fc542902e8339c23e62c692960c8cc3152776c119c23b4d0b7e91
//META: purpose=Visiting `currentScope` first means an inner declaration with the same name as an outer one shadows it inside the inner block.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Step 1 of the documented walk is "visit currentScope itself ...".
// An inner `int x` therefore shadows an outer `int x` for the
// duration of the inner block, and the outer is restored on exit.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 1;
    {
        int x = 2;
        // CHECK: inner=2
        printf("inner=%d\n", x);
    }
    // CHECK: outer=1
    printf("outer=%d\n", x);
}

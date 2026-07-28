//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-walking-order-during-lookup
//META: doc_section_digest=3d87095dd18e90d93ff4e17cadb91ec9dacff31277081232f0c892a1279af690
//META: purpose=Lookup walks to the parent scope when a name is not found in the current scope; an inner block can see an enclosing block's local.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Step 3 of the documented walk is "move to currentScope->parent and
// repeat". An inner block must therefore see names declared in an
// enclosing block.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int outer = 10;
    {
        int doubled = outer * 2;
        // CHECK: doubled=20
        printf("doubled=%d\n", doubled);
    }
}

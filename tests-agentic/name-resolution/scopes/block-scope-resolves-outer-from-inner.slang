//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-walking-order-during-lookup
//META: doc_section_digest=e8fc5df08f9fc542902e8339c23e62c692960c8cc3152776c119c23b4d0b7e91
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

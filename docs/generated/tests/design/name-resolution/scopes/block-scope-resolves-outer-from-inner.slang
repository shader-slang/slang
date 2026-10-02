//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-walking-order-during-lookup
//META: doc_section_digest=b02f65c83e89f12616da6503b3f6980da4abbbb0a70dcf5a9c00e860440cb7fb
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

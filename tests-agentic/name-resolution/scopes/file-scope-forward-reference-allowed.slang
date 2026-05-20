//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=984d93f961836cc85daa4d28cbdc53dade2964991d8474fd53b87f15b712e265
//META: purpose=The block-local `hiddenFromLookup` rule does not apply at file scope; a function may refer to another function declared later in the same file.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Empty block scope" edge case explicitly scopes the
// hiddenFromLookup mechanic to block-locals. File-scope decls are not
// hidden by it, so a forward reference between top-level functions
// is allowed.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = producer();
    // CHECK: v=99
    printf("v=%d\n", v);
}

int producer() { return 99; }

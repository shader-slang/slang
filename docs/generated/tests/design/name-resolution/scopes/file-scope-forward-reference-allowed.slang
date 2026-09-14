//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
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

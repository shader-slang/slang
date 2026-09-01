//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#block-local-shadowing
//META: doc_section_digest=fff04d2b441a5ad2e4ef387152ddc76216942a0668864eb5cdfb1c9b98e7e532
//META: purpose=The hidden-until-declared rule applies only to local variables, so a struct declared later in the same block is still found by an earlier reference.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the local-variable qualifier on the block-local hiding
// rule. A struct declared inside a function body is not a local
// variable, so the filter does not suppress it and the earlier
// declaration of `Later l` resolves. CHECK asserts the field written
// through that variable reads back as 3, which requires the type to
// have resolved before its textual declaration.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    Later l;
    l.v = 3;

    struct Later { int v; }

    // CHECK: r=3
    printf("r=%d\n", l.v);
}

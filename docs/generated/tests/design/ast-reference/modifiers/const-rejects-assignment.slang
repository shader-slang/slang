//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#parameter-direction-and-storage-class-modifiers
//META: doc_section_digest=bd95695034d63fdcecbd599f22c84217f47af88c7597c20e6f353db58735f570
//META: purpose=`const` makes a local variable immutable; assigning to it is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ConstModifier` marks the variable as a compile-time-immutable
// binding. The checker rejects any subsequent assignment with an
// l-value-of-type-const-is-not-modifiable diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    const int x = 7;
    x = 42;
    //CHECK: error
    //CHECK: E30011
}

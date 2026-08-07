//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=32b5d6cdc50501bef4e7ef2a5cc9d61e93145ad0a37a1049859831fc766c24f1
//META: purpose=A type name is not overloadable either, so a block-local typedef stops the outward walk and the same-named file-scope struct is never accumulated.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the short-circuit rule on the *type* half of the documented
// "types and variables do not accumulate" pair (the variable half is
// covered by unqualified-short-circuits-on-variable.slang). A local
// `typedef int W` hides the file-scope `struct W`. CHECK asserts the
// local is initialized from an integer literal and adds to 6, which
// only type-checks if `W` resolved to `int`.

//TEST:INTERPRET(filecheck=CHECK):

struct W { int a; }

void main()
{
    typedef int W;
    W x = 5;
    // CHECK: r=6
    printf("r=%d\n", x + 1);
}

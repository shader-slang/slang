//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=32b5d6cdc50501bef4e7ef2a5cc9d61e93145ad0a37a1049859831fc766c24f1
//META: purpose=A constructor gives a mutable this, so an unqualified assignment to a field inside __init initializes the object being constructed.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the constructor arm of the this-parameter-mode update,
// which the doc lists alongside the setter as always yielding a
// mutable `this`. The __init body writes `n` by unqualified name;
// CHECK asserts the constructed value is 4 + 1, which is only visible
// to the caller if the write went through a mutable receiver.

//TEST:INTERPRET(filecheck=CHECK):

struct C
{
    int n;
    __init(int x) { n = x + 1; }
}

void main()
{
    C c = C(4);
    // CHECK: r=5
    printf("r=%d\n", c.n);
}

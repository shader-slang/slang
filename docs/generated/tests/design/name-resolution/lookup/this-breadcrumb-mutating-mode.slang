//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=32b5d6cdc50501bef4e7ef2a5cc9d61e93145ad0a37a1049859831fc766c24f1
//META: purpose=Stepping out of a mutating-qualified method sets the This parameter mode to a mutable value, so an unqualified write to a field inside that method updates the receiver.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the this-parameter-mode update the scope walk performs when
// it leaves a callable: a [mutating] method must produce a mutable
// `this` breadcrumb. The method assigns to `count` by unqualified
// name; if the breadcrumb recorded an immutable `this` the write would
// land on a copy. CHECK asserts the receiver observed both increments.

//TEST:INTERPRET(filecheck=CHECK):

struct Counter
{
    int count;
    [mutating] void bump() { count = count + 1; }
}

void main()
{
    Counter c;
    c.count = 10;
    c.bump();
    c.bump();
    // CHECK: r=12
    printf("r=%d\n", c.count);
}

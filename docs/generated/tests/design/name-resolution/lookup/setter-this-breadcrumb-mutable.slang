//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=32b5d6cdc50501bef4e7ef2a5cc9d61e93145ad0a37a1049859831fc766c24f1
//META: purpose=A property setter gives a mutable this, so an unqualified assignment to a backing field inside the set accessor updates the receiver.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the setter arm of the this-parameter-mode update, the
// second decl kind the doc names as unconditionally mutable. The set
// accessor writes the backing field `_v` by unqualified name; CHECK
// asserts the receiver holds 12 * 2 afterwards, which requires the
// breadcrumb to have carried a mutable `this` out of the accessor.

//TEST:INTERPRET(filecheck=CHECK):

struct Holder
{
    int _v;
    property int v
    {
        get { return _v; }
        set { _v = newValue * 2; }
    }
}

void main()
{
    Holder h;
    h.v = 12;
    // CHECK: r=24
    printf("r=%d\n", h._v);
}

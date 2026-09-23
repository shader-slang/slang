//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#inheritance-control-attributes
//META: doc_section_digest=01de4dedc6faa485c9070180fa925cf74a07773479e87fd09f801152f6f74992
//META: purpose=A `[sealed]` interface can still be implemented directly by a type in the same module.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The contrast case for `[open]`: `[sealed]` is the other member of the
// inheritance-control pair, and it constrains further inheritance rather than
// forbidding conformance outright. A struct implements the sealed interface and
// CHECK asserts its method result, showing the attribute is accepted and does not
// block a direct implementation.

//TEST:INTERPRET(filecheck=CHECK):

[sealed]
interface IShape
{
    int sides();
}

struct Quad : IShape
{
    int sides() { return 4; }
}

void main()
{
    Quad q;
    //CHECK: sides=4
    printf("sides=%d\n", q.sides());
}

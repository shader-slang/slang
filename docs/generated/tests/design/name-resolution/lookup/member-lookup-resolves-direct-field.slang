//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=9574aa361d7b18b1da022473e0e53acd215b9af06dae0cb39490ca13610d03bf
//META: purpose=Member lookup on a plain DeclRefType resolves a direct field declared on the underlying type decl.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the base case of the member-lookup type dispatch: a struct
// value has a DeclRefType, so `obj.name` reaches the decl arm and finds
// the direct member. CHECK asserts both field values round-trip through
// separate writes and reads, which requires each `p.x` / `p.y` to have
// bound to its own field.

//TEST:INTERPRET(filecheck=CHECK):

struct Point
{
    int x;
    int y;
}

void main()
{
    Point p;
    p.x = 11;
    p.y = 22;
    // CHECK: p=(11,22)
    printf("p=(%d,%d)\n", p.x, p.y);
}

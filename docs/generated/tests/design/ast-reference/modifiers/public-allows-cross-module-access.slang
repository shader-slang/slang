//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#visibility-modifiers
//META: doc_section_digest=1c8e25b6beb8d6b322c1d0d2039a7c5fcba36699beabc8ab756c7f1c0d6fb97b
//META: purpose=A `public` member is accessible from outside its enclosing type and a `public` function from anywhere.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `public` is the widening counterpart of `private`: both the fields
// and the free function carrying it are reachable from ordinary code. The fields
// are written and summed through a `public` function; CHECK asserts 12, which
// requires every one of those accesses to have passed the visibility check.

//TEST:INTERPRET(filecheck=CHECK):

public struct Pt
{
    public int x;
    public int y;
}

public int sumPt(Pt p)
{
    return p.x + p.y;
}

void main()
{
    Pt p;
    p.x = 5;
    p.y = 7;
    //CHECK: sum=12
    printf("sum=%d\n", sumPt(p));
}

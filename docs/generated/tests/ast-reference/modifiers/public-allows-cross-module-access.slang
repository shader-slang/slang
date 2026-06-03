//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#visibility-modifiers
//META: doc_section_digest=52c3496b4c6eb147ea8a64ec9af718be37cc7621d3bab08948b2b5b741b87fda
//META: purpose=A `public` member is accessible from outside its enclosing type and a `public` function from anywhere.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `PublicModifier` declares the member as part of the type's public
// surface. A `public` field of a struct is reachable through the
// usual member-access path.

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

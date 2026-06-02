//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#initializers
//META: doc_section_digest=42e0c0a3aebf1644284046e5a27a45b490b9bc6e618e09f646a51067c956edd4
//META: purpose=An `__init` has access to an implicit mutable `this` parameter representing the instance being initialized.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "An initializer has access to an implicit `this` variable that
// is the instance being initialized… The `this` variable of an initializer is
// always mutable."

//TEST:INTERPRET(filecheck=CHECK):

struct Point
{
    int x, y;
    __init(int px, int py)
    {
        this.x = px;
        this.y = py;
    }
}

void main()
{
    Point p = Point(7, 13);
    // CHECK: 7
    // CHECK: 13
    printf("%d\n%d\n", p.x, p.y);
}

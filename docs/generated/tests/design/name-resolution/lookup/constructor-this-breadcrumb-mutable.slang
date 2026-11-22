//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=d1c02da07b357c8b41134fbb13a153c9451e991db3087032b3d340a5ddfdfa75
//META: purpose=A constructor body has a mutable this (the checker's this computation makes it an l-value), so an unqualified assignment to a field inside __init initializes the object being constructed.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the constructor arm of the checker's `this` computation:
// lookup records only a ThisExpr breadcrumb, and checking that
// implicit `this` treats a ConstructorDecl as an l-value. The __init body writes `n` by unqualified name;
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

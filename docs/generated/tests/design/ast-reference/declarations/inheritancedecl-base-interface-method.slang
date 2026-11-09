//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#inheritancedecl
//META: doc_section_digest=fb166f641a47111139b90d9bae931d7fe4a8eda48e9f02167ee1bcb0e48fb8ab
//META: purpose=The witness recorded on an inheritance clause is what connects a requirement to the conformer's member at each dispatch site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the conformance recorded by an inheritance clause routes an
// interface requirement to the conformer's member. A struct with two fields
// implements the single requirement in terms of both; the CHECK asserts their
// product, which is only produced by the conformer's body being selected.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape
{
    int area();
}

struct Rect : IShape
{
    int w; int h;
    int area() { return w * h; }
}

int areaOf<T : IShape>(T s) { return s.area(); }

void main()
{
    Rect r;
    r.w = 4; r.h = 5;
    //CHECK: 20
    printf("%d\n", areaOf(r));
}

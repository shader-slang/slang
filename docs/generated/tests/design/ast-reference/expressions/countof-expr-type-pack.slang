//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=A CountOfExpr `countof(T)` yields the number of elements in a static type pack.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CountOfExpr is documented as `countof(T)` (element count of a
// static type pack / tuple). We use a variadic int generic and call
// countof on the parameter pack inside the generic.

//TEST:INTERPRET(filecheck=CHECK):

struct Dims<let each D : int>
{
    static const int n = countof(D);
}

void main()
{
    static const int n = Dims<10, 20, 30, 40>.n;
    // CHECK: n=4
    printf("n=%d\n", n);
}

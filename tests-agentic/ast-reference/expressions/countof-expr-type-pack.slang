//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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

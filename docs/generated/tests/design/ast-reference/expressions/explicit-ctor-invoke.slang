//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=An ExplicitCtorInvokeExpr `T(...)` is a constructor invocation; it returns a value of type T built from the arguments.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ExplicitCtorInvokeExpr inherits InvokeExpr and represents an
// explicit `T(...)` constructor call. We define a user constructor
// on a struct and invoke it; the resulting fields must reflect the
// constructor's assignments.

//TEST:INTERPRET(filecheck=CHECK):

struct Box
{
    int w;
    int h;

    __init(int wi, int hi) { w = wi; h = hi; }
}

void main()
{
    Box b = Box(3, 5);
    // CHECK: w=3 h=5
    printf("w=%d h=%d\n", b.w, b.h);
}

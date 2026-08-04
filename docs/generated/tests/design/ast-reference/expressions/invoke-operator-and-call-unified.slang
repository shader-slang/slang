//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#invokeexpr-and-the-call-operator-cast-unification
//META: doc_section_digest=159873da063c0de0bb81f96fd4bec64b70eb1664f509af0a10c8eee0b3e19e7f
//META: purpose=Operator application is unified with function call: an overloaded `operator+` on a struct is invoked by writing `a + b`, the same way as a function call.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says InvokeExpr is the post-resolution form of explicit
// calls AND operator applications. Defining `operator+` on a struct
// and observing `a + b` invoke it (returning the user-defined sum)
// shows the operator and the call share an AST representation.

//TEST:INTERPRET(filecheck=CHECK):

struct V2 { int x; int y; }

V2 operator+(V2 a, V2 b)
{
    V2 r = { a.x + b.x, a.y + b.y };
    return r;
}

void main()
{
    V2 a = {1, 2};
    V2 b = {10, 20};
    V2 c = a + b;
    // CHECK: c=(11, 22)
    printf("c=(%d, %d)\n", c.x, c.y);
}

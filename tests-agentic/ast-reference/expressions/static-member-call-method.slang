//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#memberexpr-staticmemberexpr-derefmemberexpr
//META: doc_section_digest=a8293cbfbfbb580f987e71889bbd1c8f5697b1440fa8a8bc081a878f12d3aabf
//META: purpose=A StaticMemberExpr can also name a static method on a type; calling `T.f()` invokes the method without an instance.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says StaticMemberExpr is `T::m` (or `T.m` on a type
// expression). Calling a static method via `T.method()` invokes it
// and returns its value.

//TEST:INTERPRET(filecheck=CHECK):

struct Math
{
    static int add3(int x) { return x + 3; }
}

void main()
{
    int v = Math.add3(10);
    // CHECK: v=13
    printf("v=%d\n", v);
}

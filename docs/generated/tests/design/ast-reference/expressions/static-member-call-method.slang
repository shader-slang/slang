//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#memberexpr--staticmemberexpr--derefmemberexpr
//META: doc_section_digest=2e4c0cf09ff9c6fb2f3bd8e1c643d300367d45168a8b35d087ed154695482428
//META: purpose=A StaticMemberExpr can also name a static method on a type; calling `T.f()` invokes the method without an instance.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a static method can be named through its type and called
// without an instance. `Counter.next()` is invoked twice from a free function;
// the CHECK reads the returned values, which only exist if the type-qualified
// name resolved to the static method.

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

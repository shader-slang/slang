//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=2b2f6df280d876721b915b8db1e96bed9f1bfdb17423498342a13971e8db1e4d
//META: purpose=FuncType at the smallest parameter arity (0 parameters) is still a valid function type; the callable returns its result.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the small edge of FuncType: a 0-parameter
// callable surface. The FuncType's "param-types" operand is empty
// and only the result type carries information. We exercise it
// through an IFunc-like idiom to survive INTERPRET.

//TEST:INTERPRET(filecheck=CHECK):

interface ICallable0 { int call(); }
struct Const42 : ICallable0 { int call() { return 42; } }
int invokeZero<F : ICallable0>(F f) { return f.call(); }

void main()
{
    Const42 c;
    // CHECK: zero=42
    printf("zero=%d\n", invokeZero(c));
}

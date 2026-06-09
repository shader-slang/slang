//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=2b2f6df280d876721b915b8db1e96bed9f1bfdb17423498342a13971e8db1e4d
//META: purpose=FuncType at "many parameters" (>=8) holds the full param-types list; the callable can be invoked with all 8 args and produces their sum.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe at the upper end of FuncType arity: an 8-parameter
// callable surface. The FuncType's "param-types" operand carries
// 8 entries; the documented sum 1+2+...+8 = 36 confirms positional
// ordering is preserved.

//TEST:INTERPRET(filecheck=CHECK):

interface ICallable8 { int call(int a, int b, int c, int d, int e, int f, int g, int h); }
struct Sum8 : ICallable8 { int call(int a, int b, int c, int d, int e, int f, int g, int h) { return a+b+c+d+e+f+g+h; } }
int invokeEight<F : ICallable8>(F f) { return f.call(1,2,3,4,5,6,7,8); }

void main()
{
    Sum8 s;
    // CHECK: eight=36
    printf("eight=%d\n", invokeEight(s));
}

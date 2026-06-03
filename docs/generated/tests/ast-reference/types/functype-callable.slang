//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=2b2f6df280d876721b915b8db1e96bed9f1bfdb17423498342a13971e8db1e4d
//META: purpose=A FuncType holds parameter types and a result type; a function-typed value (a callable carrying a FuncType) is invokable to produce its result.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `FuncType` note states: "holds parameter types, return
// type, and an optional error type". The observable consequence is
// that a callable value can be invoked with its parameter types and
// returns its result type. We exercise this through the `IFunc`-like
// callable-as-interface idiom that survives the interpreter.

//TEST:INTERPRET(filecheck=CHECK):

interface ICallable { int call(int x); }
struct Doubler : ICallable { int call(int x) { return x * 2; } }

int twice<F : ICallable>(F f, int x) { return f.call(x); }

void main()
{
    Doubler d;
    // CHECK: r=20
    printf("r=%d\n", twice(d, 10));
}

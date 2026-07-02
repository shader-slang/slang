//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=7e903f116cccaa2d6cdd7b7b3629e179d6417c0f583ea8c8d54a18facbc96121
//META: purpose=A FuncType holds parameter types and a result type; a function-typed value is invokable to produce its result.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies FuncType's claim that it holds parameter types and a
// return type: a callable value can be invoked with its parameter
// type and returns its result type. The IFunc-like callable-as-
// interface idiom survives the interpreter; CHECK asserts the
// invoked result.

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

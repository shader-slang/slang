//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#functype
//META: doc_section_digest=5e92206b0f0b0b7cbc40b1055a81166a8628747c755d5cf61ab0cfb9b61096cf
//META: purpose=A FuncType holds parameter types followed by a result type; a value of a callable type is invokable to produce its result.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the parameter-then-result shape of a callable type by
// passing a callable value through a generic that only knows it is
// callable with an int and yields an int. CHECK asserts the invoked
// result, which requires both the parameter type and the result type to
// have travelled with the value.

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

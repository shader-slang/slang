//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#syntax
//META: doc_section_digest=53368bceb09c2fd2d073113b9d1fb5324abcb9db853a9b3aded020d2ada84c45
//META: purpose=A generic interface declared with `<...>` parameters can be conformed to at a concrete argument and its requirement called.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the interface-declaration grammar production: `interface IBox<T>`
// carries a generic parameter T used in a requirement `T get()`. A struct
// conforms at T=int and a generic caller invokes the requirement through the
// interface. CHECK asserts the stored value (7) is returned, proving the
// generic interface parameter threaded through conformance and dispatch.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IBox<T>
{
    T get();
}

struct IntBox : IBox<int>
{
    int v;
    int get() { return v; }
}

int unbox<B : IBox<int>>(B b)
{
    return b.get();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IntBox b = { 7 };
    outputBuffer[0] = unbox(b);
}
// CHECK: 7

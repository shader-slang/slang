//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#syntax
//META: doc_section_digest=d9837133963c8192c9a1fa858fc8a92c137b9d6ec976bc1f03e6d414c3a55971
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

//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#parameter-binding
//META: doc_section_digest=5fefa3cf4076961bde4d84e979866a0fe46244475e81339e0cd6922ed4777a7a
//META: purpose=Explicit binding of an otherwise-ambiguous non-fundamental type parameter (`testFunc<IBase>(a, b)`) resolves the ambiguity and compiles.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Implicit parameter binding, ambiguous type argument"
// example under its resolving form: two distinct conformers A and B of IBase
// would make T ambiguous by inference, so the call binds T explicitly as
// `testFunc<IBase>(a, b)`. The explicit argument makes the program specialize
// and compile; the entry point then writes a sentinel it could only reach if
// the call type-checked. CHECK asserts 1, proving the explicit binding resolved
// the otherwise-ambiguous non-fundamental type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IBase
{
}

struct A : IBase
{
}

struct B : IBase
{
}

void testFunc<T>(T x, T y)
{
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    A a;
    B b;
    testFunc<IBase>(a, b);
    outputBuffer[0] = 1;
}
// CHECK: 1

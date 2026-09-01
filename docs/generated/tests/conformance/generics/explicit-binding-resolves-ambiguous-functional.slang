//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#implicit-parameter-binding-ambiguous-type-argument
//META: doc_section_digest=5a242a0b2f3a0ce246085c303baec4847733bd0badeaee717cbb03060d2a52a9
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

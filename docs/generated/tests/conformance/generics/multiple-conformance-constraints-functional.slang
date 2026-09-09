//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#syntax
//META: doc_section_digest=53368bceb09c2fd2d073113b9d1fb5324abcb9db853a9b3aded020d2ada84c45
//META: purpose=A conformance constraint may list several constraining types (`where T : IA, IB`), and the optional `typename` keyword may introduce the type parameter.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies two grammar options together: the type parameter is spelled with the
// optional `typename` keyword, and one where-clause names a comma-separated
// list of constraining types. The body calls a requirement from each interface,
// which type-checks only if both conformances were recorded. A struct
// implementing both is passed in; CHECK asserts 34 (a() = 3 in the tens place,
// b() = 4 in the ones place), proving both requirements dispatched.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface IA
{
    int a();
}

interface IB
{
    int b();
}

struct S : IA, IB
{
    int a() { return 3; }
    int b() { return 4; }
}

int both<typename T>(T x) where T : IA, IB
{
    return x.a() * 10 + x.b();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    outputBuffer[0] = both(s);
}
// CHECK: 34

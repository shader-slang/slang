//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#syntax
//META: doc_section_digest=53368bceb09c2fd2d073113b9d1fb5324abcb9db853a9b3aded020d2ada84c45
//META: purpose=A generic value parameter can be declared with the traditional `type name` syntax (generic-value-param-trad-decl) instead of `let name : type`.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the traditional value-parameter grammar production: `struct
// Buf<uint N>` declares N with the `simple-type-spec identifier` form (no
// `let`). Specialized as `Buf<4>` its field is a 4-element array. CHECK asserts
// the summed elements (4) are stored, proving the traditional value-parameter
// syntax was accepted and bound N=4.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

struct Buf<uint N>
{
    int v[N];
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Buf<4> b;
    for (int i = 0; i < 4; ++i)
        b.v[i] = 1;
    outputBuffer[0] = b.v[0] + b.v[1] + b.v[2] + b.v[3];
}
// CHECK: 4

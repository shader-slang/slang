//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#syntax
//META: doc_section_digest=d9837133963c8192c9a1fa858fc8a92c137b9d6ec976bc1f03e6d414c3a55971
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

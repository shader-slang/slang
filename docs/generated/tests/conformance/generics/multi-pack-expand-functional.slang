//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/language-reference/generics.md#type-param-packs
//META: doc_section_digest=1d7c4d9f36d4d30dc9d8e89006174f0691c6f26a9618f3cb1da30611dc1fe7df
//META: purpose=Two function parameter packs of equal length expand pairwise, iterating each-expressions in lockstep.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's "Multiple function parameter packs" example plus the rule
// that multiple each-expressions in one expand iterate in lockstep: `dotProduct
// <each T>(expand each T a, expand each T b)` expands `dotHelper(each a, each b,
// r)` pairwise over two length-3 packs. CHECK asserts 4*2 + 5*1 + 6*0 == 13,
// proving the two packs advanced together element by element.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void dotHelper(float a, float b, inout float ret)
{
    ret += a * b;
}

float dotProduct<each T>(expand each T a, expand each T b) where T == float
{
    float r = 0.0f;
    expand dotHelper(each a, each b, r);
    return r;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a[3] = { 4, 5, 6 };
    float b[3] = { 2, 1, 0 };
    outputBuffer[0] = dotProduct(a[0], a[1], a[2], b[0], b[1], b[2]);
}
// CHECK: 13.0

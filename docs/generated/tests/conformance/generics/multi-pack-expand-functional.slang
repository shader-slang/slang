//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/language-reference/generics.md#multiple-function-parameter-packs
//META: doc_section_digest=6a49090c102ac71d081efba7c41258e3755cb9dfca2d7c28d44430ef49218974
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

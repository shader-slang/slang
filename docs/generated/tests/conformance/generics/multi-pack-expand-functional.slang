//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#type-param-packs
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=Multiple function parameter packs must all have equal lengths; `expand dotProductHelper(each a, each b, r)` pairwise expands two float packs of the same length.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// From the doc Examples section (Multiple function parameter packs).
// Two packs `a` and `b` must have equal lengths; `expand` iterates over both simultaneously.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void dotProductHelper(float a, float b, inout float ret) { ret += a * b; }

float dotProduct<each T>(expand each T a, expand each T b)
    where T == float
{
    float r = 0.0f;
    expand dotProductHelper(each a, each b, r);
    return r;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=8)
RWStructuredBuffer<double> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    // dot([4,5,6],[2,1,0]) = 8+5+0 = 13
    outputBuffer[0] = (double)dotProduct(4.0f, 5.0f, 6.0f, 2.0f, 1.0f, 0.0f);
}
// CHECK: 13

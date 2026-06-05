//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A constraint on a type parameter pack applies to every type in the pack; `where T == float` makes all elements of type float and allows arithmetic on each.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `where T == float` constraint on a type parameter pack applies to every element.
// `sumFloatPack(1.0f, 2.0f, 3.0f)` expands the pack of three floats and accumulates them;
// CHECK asserts 6, which is only possible if all elements satisfied the constraint and were summed.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

void addToFloat(inout float acc, float v) { acc += v; }

// Constraint on type pack: every T must equal float
float sumFloatPack<each T>(expand each T terms) where T == float
{
    float acc = 0.0f;
    expand addToFloat(acc, each terms);
    return acc;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    outputBuffer[0] = sumFloatPack(1.0f, 2.0f, 3.0f);
}
// CHECK: 6

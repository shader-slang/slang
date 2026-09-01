//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=b6a68a06f518f5bd44e3459811096c15e9f474df3116276f35e51d90ca68eef0
//META: purpose=`countof` of a fixed-size array reports that array's element count.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fixed-size-array half of the count query's documented domain.
// Three arrays of different lengths are declared and each count is written to
// its own buffer slot, so the asserted 1 / 3 / 7 sequence distinguishes a real
// per-array count from any single constant the query might fall back to.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float one[1] = { 9 };
    float three[3] = { 1, 2, 3 };
    float seven[7] = { 1, 2, 3, 4, 5, 6, 7 };
    outputBuffer[0] = countof(one);
    outputBuffer[1] = countof(three);
    outputBuffer[2] = countof(seven);
}

// CHECK: 1
// CHECK: 3
// CHECK: 7

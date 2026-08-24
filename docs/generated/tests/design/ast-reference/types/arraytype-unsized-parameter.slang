//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=An ArrayExpressionType may be unsized; a `T[]` parameter accepts sized arrays of different extents and recovers each one's count at the call site.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unsized form of the array type. One `int[]` parameter
// accepts both an int[3] and an int[5], and getCount() reports each
// caller's extent. Both sums are written to an output buffer; CHECK
// asserts 6 and 50, which differ only because the count travelled with
// the argument rather than being fixed by the parameter type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outBuf
RWStructuredBuffer<int> outBuf;

int total(int xs[])
{
    int s = 0;
    for (int i = 0; i < xs.getCount(); ++i) s += xs[i];
    return s;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int a[3] = { 1, 2, 3 };
    int b[5] = { 10, 10, 10, 10, 10 };
    // CHECK: 6
    outBuf[0] = total(a);
    // CHECK: 50
    outBuf[1] = total(b);
}

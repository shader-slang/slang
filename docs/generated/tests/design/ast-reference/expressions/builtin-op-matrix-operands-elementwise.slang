//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:01:06Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=c1890a75acdc226411b7d61f4ee2807ae95bd2bb75bf036bb2da6f27662ff946
//META: purpose=The builtin-operator fast path covers matrix operands: `+` on int2x2 values produces elementwise sums.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that builtin binary operators accept matrix operands, the
// third operand shape the checker's fast path recognizes. Two int2x2
// matrices are added and all four elements are written to the output
// buffer; CHECK asserts each elementwise sum in order.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int2x2 a = int2x2(1, 2, 3, 4);
    int2x2 b = int2x2(10, 20, 30, 40);
    int2x2 s = a + b;
    // CHECK: 11
    outputBuffer[0] = s[0][0];
    // CHECK: 22
    outputBuffer[1] = s[0][1];
    // CHECK: 33
    outputBuffer[2] = s[1][0];
    // CHECK: 44
    outputBuffer[3] = s[1][1];
}

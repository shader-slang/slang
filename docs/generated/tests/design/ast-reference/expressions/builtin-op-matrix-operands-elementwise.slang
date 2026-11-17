//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=d86992208065a1cd06198a0c8298546dee4838c3ae7ea2c807513713ba87455a
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

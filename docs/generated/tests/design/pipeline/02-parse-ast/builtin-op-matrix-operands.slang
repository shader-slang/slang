//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=5749d9886287d8389af87cb5a6a536a28da48a421ac9a27f86809fca6a7b5676
//META: purpose=A builtin arithmetic operator over builtin matrix operands is rewritten by the checker and yields the correct elementwise result.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the checker's builtin-operator rewrite covers matrix
// operands, the third operand shape the doc names alongside scalars
// and vectors. Two 2x2 int matrices built from buffer inputs are
// added elementwise; CHECK asserts each element of the sum. Buffer
// inputs keep the add out of reach of the constant folder so the
// rewritten operator is exercised through IR lowering. CPU compute is
// used because the interpreter has no matrix precedent in the suite.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4):name=inBuf
RWStructuredBuffer<int> inBuf;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outBuf
RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    int2x2 a = int2x2(inBuf[0], inBuf[1], inBuf[2], inBuf[3]);
    int2x2 b = int2x2(10, 20, 30, 40);
    int2x2 s = a + b;
    // CHECK: 11
    outBuf[0] = s[0][0];
    // CHECK: 22
    outBuf[1] = s[0][1];
    // CHECK: 33
    outBuf[2] = s[1][0];
    // CHECK: 44
    outBuf[3] = s[1][1];
}

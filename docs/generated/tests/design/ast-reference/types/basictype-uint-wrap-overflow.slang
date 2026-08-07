//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=A uint BasicExpressionType wraps modulo 2^32 on overflow; uint(0xFFFFFFFF) + 1u wraps to 0.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of unsigned wrap at the top edge of the uint
// arithmetic leaf. MAX arrives through an input buffer so the constant
// folder cannot pre-compute the add, and the runtime sum is written to
// an output buffer. CHECK asserts the wrapped value 0.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[4294967295], stride=4):name=inBuf
RWStructuredBuffer<uint> inBuf;
//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outBuf
RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint m = inBuf[0];
    // CHECK: 0
    outBuf[0] = m + 1u;
}

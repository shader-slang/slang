//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#basicexpressiontype-vectorexpressiontype-matrixexpressiontype
//META: doc_section_digest=5f41283b162e626d8a46ba0564c2bfa3960821954aadd78336372f0416cb5f48
//META: purpose=A uint BasicExpressionType wraps modulo 2^32 on overflow; uint(0xFFFFFFFF) + 1 wraps to 0.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of unsigned wrap on the uint scalar
// BasicExpressionType. MAX is fed through a uniform buffer so the
// constant folder cannot pre-compute the add; the runtime result is
// written to an output buffer. CHECK asserts the wrapped value 0.

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

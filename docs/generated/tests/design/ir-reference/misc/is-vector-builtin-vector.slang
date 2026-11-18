//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:59:05+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/misc.md#type-queries-and-predicates
//META: doc_section_digest=a314589d5bf1e20e3602ad6d1cfc7cdbc66f447af11efa30fb64ff86fa52545e
//META: purpose=`__isVector<T>()` is true for a builtin vector and false for a scalar or a matrix, because the fold classifies the operand type itself rather than its element type.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `IsVector` against the other predicates' element-unwrapping rule.
// Each shape writes its own buffer slot so a wrong answer for any one shape is
// visible: a scalar and a matrix are not vectors, while two- and four-component
// vectors (including an integer vector) are.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[9 9 9 9 9], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = __isVector<float>() ? 1 : 0;
    outputBuffer[1] = __isVector<float2>() ? 1 : 0;
    outputBuffer[2] = __isVector<int4>() ? 1 : 0;
    outputBuffer[3] = __isVector<float2x2>() ? 1 : 0;
}

// CHECK: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0

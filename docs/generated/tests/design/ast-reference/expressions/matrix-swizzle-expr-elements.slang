//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that a matrix swizzle `m._m01_m10` gathers the named row/column elements into a vector in the written order.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Matrix swizzle names elements by explicit row/column coordinates rather than
// by component letters. The matrix is built from the dispatch id so the folder
// cannot precompute the gather, and the two swizzled components are stored
// separately; the CHECKs assert the off-diagonal pair in the written order, so
// a transposed or collapsed coordinate list fails.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int base = int(tid.x);
    int2x2 m = int2x2(base + 1, base + 2, base + 3, base + 4);
    int2 s = m._m01_m10;
    outputBuffer[0] = s.x;
    outputBuffer[1] = s.y;
    // HLSL: outputBuffer
}
// CHECK: {{^}}2{{$}}
// CHECK-NEXT: {{^}}3{{$}}

//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:43:23Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a vector-typed `select` lowers to a single select instruction (SPIR-V OpSelect), not an if/else.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the vector branch of `visitSelectExpr`: when the condition operand is
// NOT a BasicExpressionType (here a bool3), the lowering takes the
// `visitInvokeExpr` path and emits a `select` op rather than the scalar
// if/else short-circuit form. Operands derive from dispatch IDs to defeat
// folding. CHECK asserts the SPIR-V contains an `OpSelect` over the v3int
// result type, the distinguishing instruction for the vector path.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
RWStructuredBuffer<int3> outputBuffer;
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int3 a = int3(tid);
    int3 b = int3(tid.zyx);
    bool3 c = a < b;
    outputBuffer[0] = select(c, a, b);
}
// CHECK: OpSelect %v3int

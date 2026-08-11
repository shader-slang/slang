//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:43:23Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a scalar `?:` inside a function lowers to a temporary plus an if/else, not a single select instruction.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the scalar branch of `visitSelectExpr`: when the condition operand is
// a BasicExpressionType and we are inside a function, the lowering builds
// then/else/after blocks and a result param (short-circuiting form) rather
// than a `select` op. Operands come from dispatch IDs to defeat constant
// folding. CHECK asserts the emitted HLSL contains an `if (a < b)` with each
// arm assigning the same temporary, the if/else form the lowering produces.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
RWStructuredBuffer<int> outputBuffer;
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    int b = int(tid.y);
    outputBuffer[0] = (a < b) ? a : b;
}
// CHECK: int [[TMP:_S[0-9]+]];
// CHECK: if(a_{{[0-9]+}} < b_{{[0-9]+}})
// CHECK: [[TMP]] = a_{{[0-9]+}};
// CHECK: [[TMP]] = b_{{[0-9]+}};
// CHECK: outputBuffer_{{[0-9]+}}[int(0)] = [[TMP]];

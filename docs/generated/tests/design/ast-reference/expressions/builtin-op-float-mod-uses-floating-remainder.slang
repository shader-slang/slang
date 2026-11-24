//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:11+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#builtinoperatorexpr
//META: doc_section_digest=d86992208065a1cd06198a0c8298546dee4838c3ae7ea2c807513713ba87455a
//META: purpose=A floating-point `%` builds a BuiltinOperatorExpr whose Mod operation is treated as a floating-point remainder, for concrete float operands and for a generic parameter constrained to `__BuiltinFloatingPointType`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the checker records that the operand element type is floating
// point for `Mod`, so lowering picks the floating-point remainder. The same
// `%` is applied to a concrete float and to a `__BuiltinFloatingPointType`
// generic parameter; CPU results 1.5 / 3.5 / -1.5 show a floating remainder,
// and the SPIR-V emit shows OpFRem (an integer remainder would be OpSRem).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

T genericMod<T : __BuiltinFloatingPointType>(T a, T b)
{
    return a % b;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float a = 7.5 + float(tid.x);
    outputBuffer[0] = a % 2.0;
    outputBuffer[1] = genericMod<float>(a, 4.0);
    outputBuffer[2] = -a % 2.0;
}

// CHECK: 1.5
// CHECK-NEXT: 3.5
// CHECK-NEXT: -1.5
// SPV-DAG: OpFRem %float
// SPV-NOT: OpSRem
// HLSL-DAG: % 2.0f
// HLSL-DAG: % b_{{[0-9]+}}
// METAL-DAG: fmod

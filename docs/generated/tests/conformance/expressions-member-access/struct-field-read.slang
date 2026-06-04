//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-member-access.md#member-expression
//META: doc_section_digest=ab827430dedad798221ccb6a8d05027ec37ed381ec7b70905cec740e9ed7e864
//META: purpose=Struct field access via `base.fieldName` reads the named field; multiple fields read independently.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SRHLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRGLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRSPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRMTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRWGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SRCPP):-target cpp -entry computeMain -stage compute

// Verifies `base.fieldName` reads return the named field's value.
// Writes three distinct field values to an output buffer and asserts
// the buffer receives 3/5/7 in field order, proving x/y/z read independence.
// Each SIMPLE directive checks that the emitted code preserves the struct
// field access pattern, unblocking Metal and WGSL which reject printf.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct Point { int x; int y; int z; }

[numthreads(1, 1, 1)]
void computeMain()
{
    Point p = { 3, 5, 7 };
    outputBuffer[0] = p.x;
    outputBuffer[1] = p.y;
    outputBuffer[2] = p.z;
}

// CHECK: type: int32_t
// CHECK-NEXT: 3
// CHECK-NEXT: 5
// CHECK-NEXT: 7

// SRHLSL: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRHLSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRHLSL-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

// SRGLSL: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRGLSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRGLSL-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

// SRSPV: %int_3 = OpConstant %int 3
// SRSPV-DAG: %int_5 = OpConstant %int 5
// SRSPV-DAG: %int_7 = OpConstant %int 7

// SRMTL: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRMTL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRMTL-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

// SRWGSL: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRWGSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRWGSL-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

// SRCUDA: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRCUDA-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRCUDA-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

// SRCPP: x_{{[0-9]+}}
// SRCPP-DAG: y_{{[0-9]+}}
// SRCPP-DAG: z_{{[0-9]+}}

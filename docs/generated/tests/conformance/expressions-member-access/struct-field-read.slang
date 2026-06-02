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

//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=SRHLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SRGLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SRSPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=SRCUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=SRCPP):-target cpp -entry main -stage compute

struct Point { int x; int y; int z; }

void main()
{
    Point p = { 3, 5, 7 };
    //CHECK: x=3 y=5 z=7
    printf("x=%d y=%d z=%d\n", p.x, p.y, p.z);
}

// Emission checks: struct field reads appear in emitted text as `p_N.x_N` / `p_N.y_N` / `p_N.z_N`.
// SPIRV folds struct fields into individual constants but still preserves the three int values.
// Metal and WGSL do not support printf in compute, so those targets are unsupported-on-target.

// SRHLSL: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRHLSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRHLSL-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

// SRGLSL: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRGLSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRGLSL-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

// SRSPV: %int_3 = OpConstant %int 3
// SRSPV-DAG: %int_5 = OpConstant %int 5
// SRSPV-DAG: %int_7 = OpConstant %int 7

// SRCUDA: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRCUDA-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRCUDA-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

// SRCPP: p_{{[0-9]+}}.x_{{[0-9]+}}
// SRCPP-DAG: p_{{[0-9]+}}.y_{{[0-9]+}}
// SRCPP-DAG: p_{{[0-9]+}}.z_{{[0-9]+}}

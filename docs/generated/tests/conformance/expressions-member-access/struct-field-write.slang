//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-06-01T17:00:00+00:00
//META: source_commit=d25453d7f0b4867db4cb5eabf34fb6cd088cf596
//META: doc_ref=docs/language-reference/expressions-member-access.md#member-expression
//META: doc_section_digest=ab827430dedad798221ccb6a8d05027ec37ed381ec7b70905cec740e9ed7e864
//META: purpose=A member ref is an l-value when the base is an l-value and the member is mutable. Verify by assigning to a struct field.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//TEST:INTERPRET(filecheck=CHECK):
//TEST:SIMPLE(filecheck=SWHLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SWGLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SWSPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=SWCUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=SWCPP):-target cpp -entry main -stage compute

struct Point { int x; int y; }

void main()
{
    Point p = { 1, 2 };
    p.x = 100;
    p.y = 200;
    //CHECK: x=100 y=200
    printf("x=%d y=%d\n", p.x, p.y);
}

// Emission checks: struct field l-value write appears as `p_N.x_N = ...` in HLSL/GLSL,
// `(&p_N)->x_N = ...` in CUDA/CPP. SPIRV constant-folds the assigns away; the written
// values 100/200 are still visible as OpConstant. Metal/WGSL unsupported-on-target (no printf).

// SWHLSL: p_{{[0-9]+}}.x_{{[0-9]+}} = {{.*}}100
// SWHLSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}} = {{.*}}200

// SWGLSL: p_{{[0-9]+}}.x_{{[0-9]+}} = 100
// SWGLSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}} = 200

// SWSPV: %int_100 = OpConstant %int 100
// SWSPV-DAG: %int_200 = OpConstant %int 200

// SWCUDA: x_{{[0-9]+}} = int(100)
// SWCUDA-DAG: y_{{[0-9]+}} = int(200)

// SWCPP: x_{{[0-9]+}} = int(100)
// SWCPP-DAG: y_{{[0-9]+}} = int(200)

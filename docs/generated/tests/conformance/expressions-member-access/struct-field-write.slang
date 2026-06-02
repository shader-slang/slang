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

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=SWHLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SWGLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SWSPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SWMTL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SWWGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SWCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SWCPP):-target cpp -entry computeMain -stage compute

// Verifies that a member ref is an l-value when the base is an l-value and the member is mutable.
// Two struct fields are written after construction and the results are read back into the
// output buffer; CHECK asserts both written values arrive, proving the l-value write round-trip.
// Emission directives cover every text-emit back-end including Metal and WGSL.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct Point { int x; int y; }

[numthreads(1, 1, 1)]
void computeMain()
{
    Point p = { 1, 2 };
    p.x = 100;
    p.y = 200;
    outputBuffer[0] = p.x;
    outputBuffer[1] = p.y;
}

// CHECK: type: int32_t
// CHECK-NEXT: 100
// CHECK-NEXT: 200

// SWHLSL: p_{{[0-9]+}}.x_{{[0-9]+}} = {{.*}}100
// SWHLSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}} = {{.*}}200

// SWGLSL: p_{{[0-9]+}}.x_{{[0-9]+}} = 100
// SWGLSL-DAG: p_{{[0-9]+}}.y_{{[0-9]+}} = 200

// SWSPV: %int_100 = OpConstant %int 100
// SWSPV-DAG: %int_200 = OpConstant %int 200

// SWMTL: x_{{[0-9]+}} = int(100)
// SWMTL-DAG: y_{{[0-9]+}} = int(200)

// SWWGSL: x_{{[0-9]+}} = i32(100)
// SWWGSL-DAG: y_{{[0-9]+}} = i32(200)

// SWCUDA: x_{{[0-9]+}} = int(100)
// SWCUDA-DAG: y_{{[0-9]+}} = int(200)

// SWCPP: x_{{[0-9]+}} = int(100)
// SWCPP-DAG: y_{{[0-9]+}} = int(200)

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=`castFloatToInt` lowers to `OpConvertFToS`/`OpConvertFToU` keyed by the target integer's signedness, and `castIntToFloat` lowers to `OpConvertSToF`/`OpConvertUToF` keyed by the source integer's signedness.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the truncating `castFloatToInt` and the widening
// `castIntToFloat` are each a single IR opcode. Each
// splits in SPIR-V on the integer side's signedness: float->signed emits
// `OpConvertFToS`, float->unsigned `OpConvertFToU`, signed->float
// `OpConvertSToF`, unsigned->float `OpConvertUToF`. All four cells are
// distinct; CHECK pins each. Operands are uniforms so the casts survive.
//
// This is the one conversion claim the shader-source targets state just as
// explicitly as SPIR-V does, because each of them spells the conversion as
// a named constructor whose name *is* the signedness: HLSL, GLSL and Metal
// emit `int(...)` / `uint(...)` / `float(...)`, and WGSL emits
// `i32(...)` / `u32(...)` / `f32(...)`. All four cells stay distinct on
// every target, so each target pins all four.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

uniform float f;
uniform int si;
uniform uint ui;
uniform RWStructuredBuffer<int> ibuf;
uniform RWStructuredBuffer<uint> ubuf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    ibuf[0] = int(f);
    ubuf[0] = uint(f);
    fbuf[0] = float(si);
    fbuf[1] = float(ui);
}

// SPV-DAG: OpConvertFToS %int %{{[0-9]+}}
// SPV-DAG: OpConvertFToU %uint %{{[0-9]+}}
// SPV-DAG: OpConvertSToF %float %{{[0-9]+}}
// SPV-DAG: OpConvertUToF %float %{{[0-9]+}}

// HLSL-DAG: = int({{.*}}f_{{[0-9]+}});
// HLSL-DAG: = uint({{.*}}f_{{[0-9]+}});
// HLSL-DAG: = float({{.*}}si_{{[0-9]+}});
// HLSL-DAG: = float({{.*}}ui_{{[0-9]+}});

// GLSL-DAG: = int({{.*}}f_{{[0-9]+}});
// GLSL-DAG: = uint({{.*}}f_{{[0-9]+}});
// GLSL-DAG: = float({{.*}}si_{{[0-9]+}});
// GLSL-DAG: = float({{.*}}ui_{{[0-9]+}});

// METAL-DAG: = int({{.*}}f_{{[0-9]+}});
// METAL-DAG: = uint({{.*}}f_{{[0-9]+}});
// METAL-DAG: = float({{.*}}si_{{[0-9]+}});
// METAL-DAG: = float({{.*}}ui_{{[0-9]+}});

// WGSL-DAG: = i32({{.*}}f_{{[0-9]+}});
// WGSL-DAG: = u32({{.*}}f_{{[0-9]+}});
// WGSL-DAG: = f32({{.*}}si_{{[0-9]+}});
// WGSL-DAG: = f32({{.*}}ui_{{[0-9]+}});

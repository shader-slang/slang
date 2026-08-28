//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#why-this-module-exists
//META: doc_section_digest=35262d5707ecd67036d365ed617e44cd2adc36e6ef084f2995306197c22f7383
//META: purpose=A layout IR module is built per target, so the same source yields layout instructions and layout decorations on every text-emit target.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that layout-IR construction is a per-target step by compiling
// one unchanged source for seven back-ends and dumping IR for each. Each
// prefix asserts the same structural pieces - a varLayout, an offset
// operand, an EntryPointLayout and a [layout(...)] on the global_param.
// Only the structure is checked: the numeric operands inside `offset(...)`
// differ per target, which is exactly why the module is per-target.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -dump-ir -o /dev/null -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// SPV-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// SPV-DAG: varLayout(
// SPV-DAG: offset(
// SPV-DAG: EntryPointLayout(
// SPV-DAG: [layout(%{{[0-9]+}})]

// HLSL-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// HLSL-DAG: varLayout(
// HLSL-DAG: offset(
// HLSL-DAG: EntryPointLayout(
// HLSL-DAG: [layout(%{{[0-9]+}})]

// GLSL-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// GLSL-DAG: varLayout(
// GLSL-DAG: offset(
// GLSL-DAG: EntryPointLayout(
// GLSL-DAG: [layout(%{{[0-9]+}})]

// METAL-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// METAL-DAG: varLayout(
// METAL-DAG: offset(
// METAL-DAG: EntryPointLayout(
// METAL-DAG: [layout(%{{[0-9]+}})]

// WGSL-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// WGSL-DAG: varLayout(
// WGSL-DAG: offset(
// WGSL-DAG: EntryPointLayout(
// WGSL-DAG: [layout(%{{[0-9]+}})]

// CUDA-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// CUDA-DAG: varLayout(
// CUDA-DAG: offset(
// CUDA-DAG: EntryPointLayout(
// CUDA-DAG: [layout(%{{[0-9]+}})]

// CPP-LABEL: ### AFTER {{[A-Za-z0-9_]+}}:
// CPP-DAG: varLayout(
// CPP-DAG: offset(
// CPP-DAG: EntryPointLayout(
// CPP-DAG: [layout(%{{[0-9]+}})]

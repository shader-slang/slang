//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a `float3x3(scalar)` (MakeMatrixFromScalar) emits a 9-arg flat constructor on HLSL/GLSL but a single-arg `makeMatrix` on CUDA.
//META: intent=characterization
//META: covers=source/slang/slang-emit-c-like.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `kIROp_MakeMatrixFromScalar` arm in emitInstExpr
// (slang-emit-c-like.cpp:2436): it expands the scalar into one operand
// per matrix element (rows*cols = 9 for a 3x3), so HLSL/GLSL get a
// flat constructor with the scalar repeated nine times. CUDA does NOT
// take that arm — it has a prelude `makeMatrix` template, so it emits a
// single-arg call. The CHECKs pin both shapes. A `uniform`-derived
// scalar (from tid.x) defeats constant folding so the constructor
// survives to emit; the value is sunk into outputBuffer so DCE keeps it.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float s = (float)tid.x + 1.0;
    float3x3 m = float3x3(s);
    outputBuffer[0] = m[1][1];
}

// HLSL repeats the scalar nine times in a flat float3x3 constructor.
//HLSL: float3x3({{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}})
// GLSL likewise repeats the scalar nine times in a flat mat3x3 constructor.
//GLSL: mat3x3({{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}})
// Metal also fans out into a flat matrix<float,3,3> constructor.
//METAL: matrix<float,int(3),int(3)> ({{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}})
// WGSL also fans out into a flat mat3x3<f32> constructor.
//WGSL: mat3x3<f32>({{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}}, {{.*}})
// CUDA keeps it as a single-argument makeMatrix template call.
//CUDA: makeMatrix<float, 3, 3>

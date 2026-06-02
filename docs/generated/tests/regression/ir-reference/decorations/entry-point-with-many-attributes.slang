//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#entry-point-and-stage
//META: doc_section_digest=a78fa1c65d9e97005484d3326f0e07d9af56a6dadaad4955b0d779effe950727
//META: purpose=Stress - a compute entry point stacked with numthreads, WaveSize, and earlydepthstencil simultaneously surfaces all four entry-point/stage decorations (entryPoint, numThreads, waveSize, earlyDepthStencil) on the same function.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress - the entry-point-and-stage doc section lists several
// independent decorations that can coexist on a single function
// inst. Stack four user attributes on `main` ([numthreads],
// [WaveSize], [earlydepthstencil], plus the implicit entry-point
// surface) and confirm all four decorations end up on the same
// IRFunc.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(8, 4, 2)]
[WaveSize(32)]
[earlydepthstencil]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK-DAG: [entryPoint({{[0-9]+}} : Int, "main", "{{.*}}")]
// CHECK-DAG: [numThreads(8 : Int, 4 : Int, 2 : Int)]
// CHECK-DAG: [waveSize(32 : Int)]
// CHECK-DAG: [earlyDepthStencil]
// CHECK: func %main

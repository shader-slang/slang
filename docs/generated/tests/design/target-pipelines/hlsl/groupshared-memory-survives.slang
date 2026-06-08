//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
//META: purpose=A groupshared array survives the pipeline as `groupshared T name[N];` on HLSL emit.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL's `groupshared` qualifier is the compute-stage TGSM
// allocator. The HLSL emitter preserves it; the spelling is
// distinct from GLSL's `shared` and WGSL's `var<workgroup>`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

groupshared int sharedBuf[8];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    sharedBuf[gid.x] = int(tid.x);
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = sharedBuf[(gid.x + 1) % 8];
}

// CHECK: groupshared int{{.*}} sharedBuf_{{[0-9]+}}[

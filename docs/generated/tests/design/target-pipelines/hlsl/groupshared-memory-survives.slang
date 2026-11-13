//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=affc65088b4faddcbee9b222f5a894089c99cb8eee13a5f0c0bac210c0fd5c78
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

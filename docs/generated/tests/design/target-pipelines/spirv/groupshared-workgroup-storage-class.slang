//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
//META: purpose=A groupshared module-scope array lowers to a Workgroup storage class OpVariable in SPIR-V.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V's Workgroup storage class corresponds to HLSL
// groupshared / GLSL shared. Slang's emit places the variable
// in Workgroup-storage with a pointer type to match.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

groupshared int sharedMem[64];
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint gi : SV_GroupIndex)
{
    sharedMem[gi] = int(tid.x);
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = sharedMem[(gi + 1) % 64];
}

// CHECK: %sharedMem = OpVariable %{{[a-zA-Z_0-9]+}} Workgroup

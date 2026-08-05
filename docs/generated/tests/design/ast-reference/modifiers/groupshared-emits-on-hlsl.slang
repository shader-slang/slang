//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers
//META: doc_section_digest=5bcb788bb16e79fc81babc467bf58625410f93828b43732190bf907b4e20e726
//META: purpose=The `groupshared` storage modifier survives lowering to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `groupshared` selects workgroup-shared storage rather than being
// dropped during lowering. The array is written, synchronised and read back so
// it cannot be eliminated, and CHECK asserts the emitted HLSL declares it with
// the `groupshared` storage class.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

groupshared int gShared[4];

RWStructuredBuffer<int> outBuf;

[numthreads(4,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    gShared[tid.x] = int(tid.x);
    GroupMemoryBarrierWithGroupSync();
    outBuf[tid.x] = gShared[tid.x];
}

//CHECK: groupshared

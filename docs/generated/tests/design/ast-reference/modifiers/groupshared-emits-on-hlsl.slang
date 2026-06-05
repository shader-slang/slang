//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compatibility-and-hlsl-storage-class-modifiers
//META: doc_section_digest=0b146a7577c216d587114be607b7bee424fa867d45b9300854bbc44543473939
//META: purpose=The `groupshared` storage modifier survives lowering to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLGroupSharedModifier` is the modifier behind the `groupshared`
// keyword. A `groupshared` declaration must appear in the emitted
// HLSL as `groupshared`.

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

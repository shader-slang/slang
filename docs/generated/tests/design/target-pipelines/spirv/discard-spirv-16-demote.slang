//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#spir-v-specific-runtime-predicates
//META: doc_section_digest=be20aea74fa93375641ba6be403852dd450904a22e0ef9e13e36b0b0c43712d3
//META: purpose=Compiling with -profile spirv_1_6 takes the shouldEmitDiscardAsDemote=true branch — discard lowers to OpDemoteToHelperInvocation (not a terminator), and the OpKill fix-up pass is skipped.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the opposite side of the shouldEmitDiscardAsDemote
// runtime predicate. At SPIR-V 1.6+, discard becomes
// OpDemoteToHelperInvocation rather than OpKill. The
// removeUnreachableCodeAfterDiscardForOpKill fix-up is unnecessary
// because OpDemoteToHelperInvocation is not a terminator.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment -profile spirv_1_6

RWStructuredBuffer<float4> outBuf;

[shader("fragment")]
float4 main(float4 pos : SV_Position) : SV_Target
{
    if (pos.x > 0.5)
        discard;
    outBuf[0] = pos;
    return float4(1, 0, 0, 1);
}

// CHECK: OpDemoteToHelperInvocation

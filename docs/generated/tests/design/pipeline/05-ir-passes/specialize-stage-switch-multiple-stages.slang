//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=80bc5973053356e62dbd6f6ded27b7e93976729f33655c137440d5c954689e89
//META: purpose=Expansion: `__stage_switch` distinguishes vertex vs fragment branches — compiling for vertex stage observes the vertex value (10), fragment stage observes fragment value (20).
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The specialize-stage-switch pass resolves `__stage_switch` blocks
// based on the active stage. We compile the same module against two
// different entry-point stages and observe each branch.

//TEST:SIMPLE(filecheck=VERTEX):-target hlsl -stage vertex -entry vsMain
//TEST:SIMPLE(filecheck=PIXEL):-target hlsl -stage pixel -entry psMain

uniform RWStructuredBuffer<int> buf;

int pickByStage()
{
    __stage_switch
    {
        case vertex:   return 10;
        case fragment: return 20;
        default:       return 0;
    }
}

float4 vsMain(uint vid : SV_VertexID) : SV_Position
{
    buf[0] = pickByStage();
    return float4(float(vid), 0.0f, 0.0f, 1.0f);
}

float4 psMain() : SV_Target
{
    buf[0] = pickByStage();
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

// VERTEX: 10
// PIXEL: 20

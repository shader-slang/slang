//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=2f10610b6a8497f9b2cec62d411162a4585b32e9698a95e5288845967ffba16d
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

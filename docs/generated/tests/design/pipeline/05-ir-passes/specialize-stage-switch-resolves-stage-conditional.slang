//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=48bf733638e3eef5be6628f3f74ad4bfa6b0600b0268542423d3dd43367db05c
//META: purpose=The specialize-stage-switch pass selects the branch matching the active shader stage — a `__stage_switch` returns the chosen branch's value at the call site.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Specialize stage switch" row of the specialization /
// generics table (`slang-ir-specialize-stage-switch.cpp`) resolves
// `__stage_switch` blocks. In a compute entry point, the
// `compute` branch is chosen.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int pickByStage()
{
    __stage_switch
    {
        case compute: return 42;
        case vertex:  return 7;
        default:      return 0;
    }
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pickByStage();
}

// HLSL: int(42)
// HLSL-NOT: int(7)

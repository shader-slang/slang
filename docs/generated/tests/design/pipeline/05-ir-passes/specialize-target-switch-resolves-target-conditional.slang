//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=48bf733638e3eef5be6628f3f74ad4bfa6b0600b0268542423d3dd43367db05c
//META: purpose=The specialize-target-switch pass selects the branch matching the active target — different targets see different emit text.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Specialize target switch" row of the specialization /
// generics table (`slang-ir-specialize-target-switch.cpp`)
// resolves `__target_switch` blocks. A target-conditional function
// surfaces a different return value per target after the pass runs.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int pickByTarget()
{
    __target_switch
    {
        case hlsl: return 100;
        case glsl: return 200;
        default:   return 0;
    }
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pickByTarget();
}

// HLSL: 100
// GLSL: 200

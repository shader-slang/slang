//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=4af16ce22c9a66bc00390c870ab8fa13bdbb68c7437a11e64d86f8f9c02cdbe4
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

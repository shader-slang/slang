//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#targetswitchstmt-stageswitchstmt-targetcasestmt
//META: doc_section_digest=cd98281cc967041ce075594e739c4a374f2cf6163574c3a2a95a8571934a11b0
//META: purpose=Capability labels stacked in front of one body inside a `__target_switch` each select that same body, so every stacked capability compiles to the shared case body while other targets fall to `default`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `case hlsl: case glsl:` in front of a single body makes both
// capabilities pick that body. The same source is compiled for three
// targets: HLSL and GLSL must emit the shared body's value 1, while the
// CPU target matches neither label and must emit the `default` value 9.
// Three CHECK prefixes keep the per-target expectations apart.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

int helper()
{
    __target_switch
    {
    case hlsl:
    case glsl:
        return 1;
    default:
        return 9;
    }
}

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = helper();
    //HLSL: return int(1)
    //GLSL: return 1
    //CPP: return int(9)
}

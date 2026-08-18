//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=fd65eacd7dd52604642d568dd04e37acc830eba9b906dbcfc4a4d6f3d0cf9a61
//META: purpose=A `__target_switch` with a single `case` arm: under the matching target the arm body is selected by `slang-ir-specialize-target-switch.cpp` — the minimum non-empty target-switch shape.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `__target_switch` arm-count lower edge (one non-empty arm). The doc
// says specialization "resolves `[target]` ... conditional code paths
// against the active TargetRequest". A single-arm switch is the
// minimum non-empty shape; under the matching target the body is
// selected. (Other targets without an arm are exercised by the
// `target-switch-missing-arm-rejected.slang` negative companion.)

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

int pickHlslOnly()
{
    __target_switch
    {
    case hlsl:
        return 777;
    }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pickHlslOnly();
}

// HLSL: 777

//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
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

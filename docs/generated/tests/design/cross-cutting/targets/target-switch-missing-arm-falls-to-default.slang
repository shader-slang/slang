//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
//META: purpose=When the active target has no matching `case` arm in `__target_switch` but a `default:` arm exists, `slang-ir-specialize-target-switch.cpp` selects the default — the legal alternative to the negative "no arm, no default" companion.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Positive boundary companion of `target-switch-missing-arm-rejected.slang`.
// The doc names the specialization pass that resolves these switches;
// when `case hlsl` is the only specific arm and `default:` is present,
// every non-HLSL target falls to the default body and the entry-point
// capability check succeeds. We pick HLSL (case) and GLSL/CUDA
// (default) to bracket both code paths.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

int pick()
{
    __target_switch
    {
    case hlsl: return 1;
    default: return 99;
    }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pick();
}

// HLSL: return int(1)
// GLSL: return 99
// CUDA: return int(99)

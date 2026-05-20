//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
//META: purpose=Stress pattern: a `__target_switch` nested inside another `__target_switch`, both resolved by `slang-ir-specialize-target-switch.cpp` for each target — the IR pass handles recursive case-resolution.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe of the target-switch specialization pass: a switch
// nested inside another switch. The outer switch routes on the
// active target; the inner switch routes again on the same target,
// effectively forcing the IR pass to re-evaluate the same predicate.
// Each text-emit target should see the appropriate inner-arm body
// (or the outer's `default:` fallback).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute

int nested(int x)
{
    __target_switch
    {
    case hlsl:
    case glsl:
    case spirv:
        __target_switch
        {
        case hlsl: return 1000 + x;
        case glsl:
        case spirv: return 2000 + x;
        }
    case cuda: return 3000 + x;
    default: return 9000 + x;
    }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = nested(int(tid.x));
}

// HLSL: 1000
// GLSL: 2000
// SPIRV: OpConstant {{.*}} 2000
// CUDA: 3000
// METAL: 9000

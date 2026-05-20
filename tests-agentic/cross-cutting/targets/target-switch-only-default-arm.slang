//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
//META: purpose=A `__target_switch` whose only arm is `default:` provides a universal fallback; every target's `slang-ir-specialize-target-switch.cpp` lowering selects the default body.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `__target_switch` boundary: only `default:` is present. The doc's
// "How target choice affects IR" entry names the specialization pass
// that resolves these switches against the active target; with no
// explicit case arms, every target falls through to the default. The
// CHECK pattern is the constant returned by the default arm.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

int defaultOnly()
{
    __target_switch
    {
    default:
        return 555;
    }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = defaultOnly();
}

// HLSL: 555
// GLSL: 555
// SPIRV: OpConstant {{.*}} 555
// CUDA: 555
// CPP: 555

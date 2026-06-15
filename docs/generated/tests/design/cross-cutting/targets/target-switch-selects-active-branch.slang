//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
//META: purpose=`slang-ir-specialize-target-switch.cpp` resolves `[target]` switches against the active TargetRequest; a `__target_switch` with per-target branches surfaces the selected branch's body in the target's emitted text.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says target-switch specialization "resolves `[target]` /
// `[stage]` conditional code paths against the active
// `TargetRequest`". Use `__target_switch` to pick a per-target
// constant and store it; on each text-emit backend the emitted
// kernel should hold the value chosen by that backend's case arm.
//
// 100 -> HLSL, 200 -> GLSL/SPIR-V, 300 -> CUDA, 400 -> CPP.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

int pickByTarget()
{
    __target_switch
    {
    case hlsl:
        return 100;
    case glsl:
    case spirv:
        return 200;
    case cuda:
        return 300;
    case cpp:
        return 400;
    }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pickByTarget();
}

// HLSL: 100
// GLSL: 200
// SPIRV: OpConstant {{.*}} 200
// CUDA: 300
// CPP: 400

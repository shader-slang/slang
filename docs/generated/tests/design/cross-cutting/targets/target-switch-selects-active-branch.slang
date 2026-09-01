//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=fd65eacd7dd52604642d568dd04e37acc830eba9b906dbcfc4a4d6f3d0cf9a61
//META: purpose=`slang-ir-specialize-target-switch.cpp` resolves `[target]` switches against the active TargetRequest; a `__target_switch` with per-target branches surfaces the selected branch's body in the target's emitted text.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `[target]` conditional is resolved against the
// active target rather than being carried into the backend. A
// `__target_switch` returns a distinct constant per target arm and
// the result is stored to a buffer; each backend's CHECK asserts its
// own constant reaches the emitted text, which is only possible if
// specialization picked that arm for that target.
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

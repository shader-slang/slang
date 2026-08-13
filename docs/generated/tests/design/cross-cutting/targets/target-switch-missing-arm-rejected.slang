//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=fd65eacd7dd52604642d568dd04e37acc830eba9b906dbcfc4a4d6f3d0cf9a61
//META: purpose=A `__target_switch` with no arm matching the active target and no `default:` is rejected with a stage / capability diagnostic when reached from an entry point on the unmatched target.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative companion of `target-switch-single-arm.slang`. The doc's
// "How target choice affects IR" section names the specialization
// pass as the resolver for `[target]` conditional paths; when no arm
// matches the active TargetRequest and no `default:` exists, the
// reachable callee carries no implementation for that target and the
// capability check fails at the entry-point signature.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target glsl -entry main -stage compute

int pickHlslOnly()
{
    __target_switch
    {
    case hlsl:
        return 100;
    }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ entrypoint 'main' uses features that are not available in 'compute' stage for 'glsl' compilation target.
*/
//CHECK: E36107
{
    outBuf[tid.x] = pickHlslOnly();
}

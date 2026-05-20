//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=96146019aaea16661ac66159cf8328c2ae1406fa65d0708df73480a207c02e9c
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
{
    outBuf[tid.x] = pickHlslOnly();
}

//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:33:12+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#how-target-choice-affects-ir
//META: doc_section_digest=a06981cfad27799a980d12bc9debd43f1796df2e683f48b7ed9122101fbb44c3
//META: purpose=`__stage_switch` is resolved per referencing entry point's stage (not against the target's capability set), so one shared function yields the compute arm in a compute entry point and the fragment arm in a fragment entry point.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `specializeStageSwitch` takes only the IR module and specializes per
// referencing entry point's stage. One helper is shared by a compute
// and a fragment entry point; each compile selects its own stage's arm,
// and the other stage's arm body is absent from the output.

//TEST:SIMPLE(filecheck=COMP):-target hlsl -entry cmain -stage compute
//TEST:SIMPLE(filecheck=FRAG):-target hlsl -entry fmain -stage fragment
//TEST:SIMPLE(filecheck=COMPGL):-target glsl -entry cmain -stage compute

RWStructuredBuffer<int> outBuf;

int pick()
{
    __stage_switch
    {
    case compute:
        return 111;
    case fragment:
        return 222;
    default:
        return 333;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void cmain(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = pick();
}

[shader("fragment")]
float4 fmain() : SV_Target
{
    return float4(pick());
}

// COMP: 111
// COMP-NOT: 222
// FRAG: 222
// FRAG-NOT: 111
// COMPGL: 111
// COMPGL-NOT: 222

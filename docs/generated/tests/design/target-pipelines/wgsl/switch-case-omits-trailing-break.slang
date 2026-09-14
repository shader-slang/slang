//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#wgslsourceemitter-policy-overrides-and-bool-conditioned-values
//META: doc_section_digest=d47eda324078a32dbc73f59419bad83ce4ed4cd8713fc5e48372759b3592f93c
//META: purpose=shouldEmitSwitchCaseTerminatingBreak returns false for WGSL so the trailing break Slang places at the end of every switch case is omitted
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the WGSL emitter policy override that drops the redundant
// terminating `break` Slang normally appends to each switch case: reaching
// the end of a WGSL case already exits the switch, and older validators
// reject a `break` outside a loop. Every case below ends with an explicit
// `break` in the Slang source, so if the override were not honored the
// emitted text would contain the keyword. CHECK asserts the switch is
// emitted and CHECK-NOT asserts no `break` survives anywhere in the output.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int v = 0;
    switch (int(tid.x))
    {
    case 0: v = 10; break;
    case 1: v = 20; break;
    default: v = 30; break;
    }
    outBuf[tid.x] = v;
}

// CHECK: switch(
// CHECK-NOT: break

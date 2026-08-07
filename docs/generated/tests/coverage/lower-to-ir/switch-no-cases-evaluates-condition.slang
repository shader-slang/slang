//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:43:23Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=620238fbdc64454143e731292800028152e8e67e2b491ba038b7ff1979f8cb84
//META: purpose=Verifies a `switch` with no case/default labels still evaluates (and keeps the side effects of) its condition but emits no switch instruction.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the early-out in `visitSwitchStmt`: it lowers the condition first,
// then `hasSwitchCases` is false so it returns before emitting any switch.
// The side-effecting condition call must survive (its result is unused but
// the call is observable). CHECK asserts the emitted HLSL still calls
// sideEffect_0 and that no `switch(` keyword is generated.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
RWStructuredBuffer<int> outputBuffer;
int sideEffect(int v) { outputBuffer[2] = v; return v; }
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    switch (sideEffect(int(tid.x)))
    {
    }
    outputBuffer[0] = 1;
}
// CHECK: sideEffect_0(int(tid_{{[0-9]+}}.x))
// CHECK-NOT: switch(

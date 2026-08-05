//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#branch--flatten--loopcontrol
//META: doc_section_digest=f2ba884d83c46721a4e1598e9622f13a2cab5e3cbdcc2f2993805ccbe9c8f247
//META: purpose=The loopControl loop mode recorded from [loop] flows through the IR unchanged and is re-emitted by the HLSL backend as the [loop] attribute on the loop.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `loopControl` is a pass-through hint the backend turns back
// into a target construct. The HLSL CHECKs assert `[loop]` is printed
// immediately before a `for`, i.e. the compiler kept the loop rolled and
// forwarded the request rather than acting on it in the IR. The loop
// accumulates a thread-id-dependent value into a buffer so it survives to
// emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;

    [loop]
    for (int i = 0; i < 8; i++)
    {
        sum += int(tid.x) * 2 + i;
    }

    outputBuffer[tid.x] = sum;
}

// HLSL: [loop]
// HLSL: for

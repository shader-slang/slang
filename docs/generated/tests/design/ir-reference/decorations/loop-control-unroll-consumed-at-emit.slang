//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:10:36Z
//META: source_commit=f39188078fe010048714735213c24a7cb15532b9
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#branch-flatten-loopcontrol
//META: doc_section_digest=262cdb3047c0e840655e74b96e0546c7c47439596c001991c8eb9e996922be06
//META: purpose=The loopControl unroll mode recorded from [unroll] flows through the IR unchanged and is re-emitted by the HLSL backend as the [unroll] attribute on the loop.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the loopControl decoration recorded for [unroll] is a
// pass-through hint consumed at emit: the HLSL backend prints the
// [unroll] attribute back onto the for statement instead of unrolling
// the loop itself. The loop body accumulates a thread-id-dependent
// value that escapes through a buffer write, so the loop survives to
// the emit step rather than being folded away.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;

    [unroll]
    for (int i = 0; i < 8; i++)
    {
        sum += int(tid.x) + i;
    }

    outputBuffer[tid.x] = sum;
}

// HLSL: [unroll]
// HLSL: for

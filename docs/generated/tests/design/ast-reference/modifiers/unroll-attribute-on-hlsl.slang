//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#nodes
//META: doc_section_digest=29d86296deb73a57a4f207507e16ba645a6bf2e722162db8157e7554d2cf8004
//META: purpose=`[unroll]` on a loop appears as an unroll attribute in the emitted code.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `[unroll]` is a hint forwarded to the downstream compiler rather
// than something Slang consumes itself: the loop is still a loop in the emitted
// HLSL, but it now carries the attribute. CHECK asserts the `[unroll]` token
// appears in the emitted text.

// SPIR-V keeps the loop as a loop as well and forwards the hint as the
// `Unroll` loop-control mask on `OpLoopMerge`, so the SPIRV prefix shows the
// attribute reaches the Khronos back end as a hint rather than being performed
// by Slang.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main()
{
    int total = 0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        total += i;
    }
    outBuf[0] = total;
}

//CHECK: [unroll]

//SPIRV: OpLoopMerge %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Unroll

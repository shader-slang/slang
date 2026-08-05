//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=2884135594e88fad2ff6c4fdc20d6bf694888dfaa9f666f2ac51e906fc88f826
//META: purpose=`[unroll]` on a loop appears as an unroll attribute in the emitted code.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `[unroll]` is a hint forwarded to the downstream compiler rather
// than something Slang consumes itself: the loop is still a loop in the emitted
// HLSL, but it now carries the attribute. CHECK asserts the `[unroll]` token
// appears in the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

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

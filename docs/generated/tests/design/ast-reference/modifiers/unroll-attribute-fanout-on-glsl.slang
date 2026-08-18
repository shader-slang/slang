//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=d7075765bda80113ade59214eaa103fee6ef8daf55af7cdaae950e1bb4aec52e
//META: purpose=`[unroll]` on a loop appears as an unroll attribute in the emitted code.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Fan-out of the same loop-hint claim onto the GLSL back end, where the hint
// is spelled with the C++-attribute syntax. CHECK pins that token with the
// brackets escaped, since FileCheck reads a bare double bracket as a variable
// reference, proving the hint survives GLSL codegen too.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

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

//CHECK: {{\[\[}}unroll{{\]\]}}

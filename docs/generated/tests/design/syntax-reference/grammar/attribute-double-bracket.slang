//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#attributes-and-decorations
//META: doc_section_digest=15532574d330d6144dfcff6e6eb0a667f59639395aa4b4eccbd806a31f935008
//META: purpose=`Attribute` also accepts the C++-style double-bracket spelling `'[[' AttributeBody ']]'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the double-bracket alternative of the Attribute production
// carries the same meaning as the single-bracket form: the thread-group
// size written as `[[numthreads(2, 1, 1)]]` has to reach the emitted
// HLSL as `[numthreads(2, 1, 1)]`. Checking the argument values (not
// just the attribute name) is what rules out the brackets being parsed
// as an empty attribute plus a stray token.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

RWStructuredBuffer<int> outputBuffer;

// CHECK: numthreads(2, 1, 1)
[[numthreads(2, 1, 1)]]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = int(tid.x);
}

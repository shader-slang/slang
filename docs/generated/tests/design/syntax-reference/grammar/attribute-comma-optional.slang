//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#attributes-and-decorations
//META: doc_section_digest=15532574d330d6144dfcff6e6eb0a667f59639395aa4b4eccbd806a31f935008
//META: purpose=Several `AttributeBody` items may share one bracket and the separating comma is optional, so `[a b]` parses like `[a, b]`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `(','? AttributeBody)*` repetition. Two attributes are
// packed into a single bracket with no comma between them, and both
// have to take effect: `shader("compute")` is what makes the function
// an entry point at all (the compile is driven without -entry/-stage),
// and `numthreads(2, 1, 1)` has to survive as the group size. Losing
// either one changes the emitted text.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl

RWStructuredBuffer<int> outputBuffer;

// CHECK: numthreads(2, 1, 1)
// CHECK: void computeMain
[shader("compute") numthreads(2, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 1;
}

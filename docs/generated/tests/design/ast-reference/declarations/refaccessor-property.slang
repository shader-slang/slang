//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=7ab459ecbfa076cb34ec94ae11dca1b7bd65f74d607addf906138dac14f78530
//META: purpose=A RefAccessorDecl (`ref`) keeps the property assignable, and the HLSL backend compiles the assignment through it.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a property carrying an explicit `ref` accessor is still an
// l-value at the HLSL backend. The shader assigns through the property and
// reads it back into an output buffer; the CHECK asserts the entry point was
// emitted, which only happens if the ref accessor legalized successfully.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Cell
{
    int _v;
    property int value
    {
        get { return _v; }
        [mutating] ref { return _v; }
    }
}

//TEST_INPUT: set outBuf = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outBuf;

//CHECK: void main
[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    Cell c;
    c._v = 0;
    c.value = 13;
    outBuf[0] = c.value;
}

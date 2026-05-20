//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=1ed46d6cbf110c7915cfeb70b35be1654971c6062c82f1442ee85f909657d1e8
//META: purpose=A RefAccessorDecl is emitted by the HLSL backend as a return-by-reference helper; the ref accessor's body appears in the emitted text.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a property with an explicit `ref` accessor compiles
// for an HLSL backend. The user-visible consequence is that the
// property remains an l-value (assignable) on the target.

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

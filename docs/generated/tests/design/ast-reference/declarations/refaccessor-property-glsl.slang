//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=7ab459ecbfa076cb34ec94ae11dca1b7bd65f74d607addf906138dac14f78530
//META: purpose=A RefAccessorDecl (`ref`) keeps the property assignable, and the HLSL backend compiles the assignment through it.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same `ref` accessor claim on GLSL, which has no native
// references. The accessor is emitted as a helper taking the aggregate by
// `inout` and returning the backing field; the CHECKs pin the helper
// signature, its body, and the call being used as the left side of the
// assignment, so the accessor is observably still doing the write.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

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

//GLSL: Cell_value_ref_0(inout Cell_0 {{[A-Za-z0-9_]+}})
//GLSL: return {{[A-Za-z0-9_]+}}._v_{{[0-9]+}};
//GLSL: Cell_value_ref_0({{[A-Za-z0-9_]+}}) = 13;
[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    Cell c;
    c._v = 0;
    c.value = 13;
    outBuf[0] = c.value;
}

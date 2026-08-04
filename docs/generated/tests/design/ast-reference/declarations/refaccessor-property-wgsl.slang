//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#accessordecl-family
//META: doc_section_digest=7ab459ecbfa076cb34ec94ae11dca1b7bd65f74d607addf906138dac14f78530
//META: purpose=A RefAccessorDecl is emitted by the HLSL backend as a return-by-reference helper; the ref accessor's body appears in the emitted text.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the RefAccessorDecl of a property lowers to a return-by-reference
// helper on the WGSL backend. WGSL models the `ref` accessor as a helper
// `Cell_value_ref_0` returning a `ptr<function, i32>` into the backing field;
// the assignment `c.value = 13` becomes a call to that helper used as an
// l-value. CHECK pins the pointer-returning helper signature and its body so
// the ref accessor is observably present in the emitted WGSL.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

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

//WGSL: fn Cell_value_ref_0({{.*}}ptr<function, Cell_0>){{.*}}-> ptr<function, i32>
//WGSL: return {{.*}}._v_{{[0-9]+}};
//WGSL: Cell_value_ref_0({{.*}}) = i32(13);
[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    Cell c;
    c._v = 0;
    c.value = 13;
    outBuf[0] = c.value;
}

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=4af16ce22c9a66bc00390c870ab8fa13bdbb68c7437a11e64d86f8f9c02cdbe4
//META: purpose=A call to a function taking a `struct` parameter with an array field is specialized so the array is referenced from the global shader parameter directly instead of through the parameter.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the specialize-arrays claim: `lookup(Table t, int i)` takes a
// struct whose only field is a fixed-size array, and it is called with
// the module-scope `uniform Table gTable`. After specialization the
// struct parameter is gone from the callee's signature and the array
// element is read straight off the global parameter block. The CHECKs
// assert the emitted callee takes only the surviving `int` argument and
// that its body indexes `globalParams...gTable...coeffs`, which is only
// possible when the array reference was rewritten to the global.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

struct Table
{
    float coeffs[8];
};

uniform Table gTable;
uniform RWStructuredBuffer<float> buf;

float lookup(Table t, int i)
{
    return t.coeffs[i] + t.coeffs[(i + 1) % 8];
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = lookup(gTable, int(tid.x) % 8);
}

// The struct parameter is dropped; only the int argument survives.
// HLSL: float lookup_{{[0-9]+}}(int
// HLSL: globalParams_{{[0-9]+}}.gTable_{{[0-9]+}}.coeffs_{{[0-9]+}}[

// GLSL: float lookup_{{[0-9]+}}(int
// GLSL: globalParams_{{[0-9]+}}.gTable_{{[0-9]+}}.coeffs_{{[0-9]+}}[

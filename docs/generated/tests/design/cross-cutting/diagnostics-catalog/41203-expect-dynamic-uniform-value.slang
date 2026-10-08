//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=9b2d1558dfdf5d77892bbedd97bba8692c1463342a38bff41bf5d5d87684a201
//META: purpose=Fires diagnostic E41203 (expect-dynamic-uniform-value) - value must be dynamic uniform
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41203
//META: catalog_name=expect-dynamic-uniform-value
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Writing a per-thread value into a `dynamic_uniform` struct field
// triggers the uniformity-store warning under -validate-uniformity.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -profile cs_6_0 -validate-uniformity

RWStructuredBuffer<int> output;

struct B { dynamic_uniform int d; }
ConstantBuffer<B> ub;

void main(int tid : SV_DispatchThreadID) {
    B obj = ub;
    obj.d = tid;
    output[0] = obj.d;
}

//CHECK: E41203

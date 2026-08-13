//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=857d9a7b5620991cb161cbee5d5f843c16e973d3c494110ba22777ebfea3d38f
//META: purpose=The lower-optional-type pass turns `Optional<T>` into a plain struct with `value` and `hasValue` fields visible on every text target.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Optional<int>` is a Slang language type that the
// `slang-ir-lower-optional-type.cpp` pass lowers to a struct holding
// the value plus a `hasValue` boolean. The lowered struct is
// observable in the emit of every C-like text target.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

Optional<int> makeOpt(int v) { if (v > 0) return v; return none; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<int> r = makeOpt(a);
    if (r.hasValue) buf[0] = r.value;
    else buf[0] = -1;
}

// HLSL: hasValue
// GLSL: hasValue
// CUDA: hasValue
// CPP: hasValue

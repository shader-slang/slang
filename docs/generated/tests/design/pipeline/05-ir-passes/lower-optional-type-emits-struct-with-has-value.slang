//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
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

//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#spir-v-literals-and-kinds
//META: doc_section_digest=ef66016fdd706005d0d8fa43c10be7a90ee7d5b5099b8b6fab751a3d3d27cc5e
//META: purpose=Stress: an instantiation of a generic with a Vec type-argument still surfaces the type_t kind on the generic param.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// an instantiation of a generic with a Vec type-argument still
// surfaces the type_t kind on the generic param. Even when the generic
// is instantiated at a non-trivial type (`float3` here), the generic-
// param's kind in the IR dump is `type_t`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

T pass<T>(T x) { return x; }

uniform float3 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    float3 r = pass<float3>(v);
    buf[0] = r.x;
}

// IR: generic
// IR: param %T {{.*}} type_t

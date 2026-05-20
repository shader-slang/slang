//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#spir-v-literals-and-kinds
//META: doc_section_digest=454a599fd6178f1993ba645f52b6ca4c0016c5f625f396e566bfbc740683a726
//META: purpose=Stress: an instantiation of a generic with a Vec type-argument still surfaces the type_t kind on the generic param.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `type_t` represents `IRType`-valued operands such
// as generic arguments. Even when the generic is instantiated at a
// non-trivial type (`float3` here), the generic-param's kind in
// the IR dump is `type_t`.

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

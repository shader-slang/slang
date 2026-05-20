//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=094f3ccbff16c121504cf60cd04ae15b937c78df94da8d568013fe4104a433b9
//META: purpose=Stress: RWStructuredBuffer parameterised by a struct of three vector fields preserves all field rows and the struct identity in the IR type.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says `RWStructuredBuffer`'s element-type operand is any
// value type. A more complex struct (three vector fields of
// different elementCount) still composes — the buffer type names
// the struct, and the struct's `field` rows preserve each Vec.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Complex { float3 pos; float2 uv; float4 col; }

uniform RWStructuredBuffer<Complex> buf;
uniform Complex x;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = x;
}

// IR-LABEL: struct %Complex
// IR: field(%{{.*}}, Vec(Float, 3 : Int))
// IR: field(%{{.*}}, Vec(Float, 2 : Int))
// IR: field(%{{.*}}, Vec(Float, 4 : Int))
// IR: let {{.*}}%buf{{.*}}: RWStructuredBuffer(%Complex{{.*}}= global_param

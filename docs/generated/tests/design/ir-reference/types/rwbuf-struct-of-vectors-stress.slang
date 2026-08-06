//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=4995d79f1d575606442d0357f0fb015ffcc2489a1b4116d09d5d6d1d0bde2e0f
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

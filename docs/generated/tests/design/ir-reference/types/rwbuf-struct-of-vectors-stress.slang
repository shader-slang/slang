//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=Stress: RWStructuredBuffer parameterised by a struct of three vector fields preserves all field rows and the struct identity in the IR type.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// RWStructuredBuffer parameterised by a struct of three vector fields
// preserves all field rows and the struct identity in the IR type. A
// more complex struct (three vector fields of different elementCount)
// still composes — the buffer type names the struct, and the struct's
// `field` rows preserve each Vec.

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

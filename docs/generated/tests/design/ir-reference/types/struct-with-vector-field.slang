//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=b49b3d499615e99101dda545c0f7fef50e62178dcf5f11f8dcef9ff0becec030
//META: purpose=Boundary: a struct field of vector type composes; the field row carries the full Vec(...) operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the `struct` opcode's `field` children carry the
// field's IR type. A `float4 v;` field surfaces
// `field(%key, Vec(Float, 4 : Int))` — composition between the
// `struct` and `Vec` opcodes.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Holder { float4 v; }

uniform Holder s;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = s.v.x;
}

// IR-LABEL: struct %Holder
// IR: field(%{{.*}}, Vec(Float, 4 : Int))

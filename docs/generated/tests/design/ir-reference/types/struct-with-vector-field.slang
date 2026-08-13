//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=0a354de05012835336b3c276acede5bc3e74e766304e49cb482449117371f325
//META: purpose=Boundary: a struct field of vector type composes; the field row carries the full Vec(...) operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a struct field of vector type composes; the field row carries the
// full Vec(...) operand. A `float4 v;` field surfaces `field(%key,
// Vec(Float, 4 : Int))` — composition between the `struct` and `Vec`
// opcodes.

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

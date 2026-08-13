//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=0a354de05012835336b3c276acede5bc3e74e766304e49cb482449117371f325
//META: purpose=A user struct lowers to a parent `struct %Name` opcode containing `field(%key, <T>)` rows.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A user struct lowers to a parent `struct %Name` opcode containing
// `field(%key, <T>)` rows.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Mixed { int x; float y; bool z; }

uniform Mixed s;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = (s.z ? 1.0 : 0.0) + float(s.x) + s.y;
}

// IR-LABEL: struct %Mixed
// IR: field(%{{.*}}, Int)
// IR: field(%{{.*}}, Float)
// IR: field(%{{.*}}, Bool)

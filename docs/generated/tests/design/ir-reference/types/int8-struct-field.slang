//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=0a354de05012835336b3c276acede5bc3e74e766304e49cb482449117371f325
//META: purpose=Expansion: a struct field of Int8 type lowers to a field(...) row carrying Int8 as its type operand.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a struct field of Int8 type lowers to a field(...) row carrying Int8
// as its type operand. A struct with two `int8_t` fields surfaces two
// `field(%key, Int8)` rows in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Pair { int8_t a; int8_t b; }

uniform Pair p;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(p.a) + int(p.b);
}

// IR-LABEL: struct %Pair
// IR: field(%{{.*}}, Int8)

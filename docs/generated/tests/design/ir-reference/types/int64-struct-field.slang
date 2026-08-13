//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#struct-and-class-containers
//META: doc_section_digest=0a354de05012835336b3c276acede5bc3e74e766304e49cb482449117371f325
//META: purpose=Expansion: a struct field of Int64 lowers to a field(...) row carrying Int64.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// a struct field of Int64 lowers to a field(...) row carrying Int64.
// Two `int64_t` fields surface two `field(%key, Int64)` rows.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Range64 { int64_t lo; int64_t hi; }

uniform Range64 r;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(r.lo) + int(r.hi);
}

// IR-LABEL: struct %Range64
// IR: field(%{{.*}}, Int64)
